// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66WebView2Host.h"

#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2

#include "Misc/Paths.h"

// NOTE: WebView2 requires Win32 headers, but they define macros like `max`, `PlaySound`, `UpdateResource`
// which can break Unreal/standard headers when using unity builds. Wrap + undef to keep the TU clean.
#include "Windows/AllowWindowsPlatformTypes.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <objbase.h>
#include <intrin.h>

// Some Windows SDK/UE include permutations can result in WRL headers not seeing InterlockedCompareExchange.
// Provide a tiny local fallback only for the WRL include, then immediately undef it to avoid polluting the TU.
#if !defined(InterlockedCompareExchange)
static __forceinline LONG T66_InterlockedCompareExchange(volatile LONG* Destination, LONG Exchange, LONG Comparand)
{
	return _InterlockedCompareExchange(reinterpret_cast<volatile long*>(Destination), Exchange, Comparand);
}
#define InterlockedCompareExchange T66_InterlockedCompareExchange
#define T66_UNDEF_INTERLOCKED_COMPAREEXCHANGE 1
#endif

#include <wrl.h>

#if defined(T66_UNDEF_INTERLOCKED_COMPAREEXCHANGE)
#undef InterlockedCompareExchange
#undef T66_UNDEF_INTERLOCKED_COMPAREEXCHANGE
#endif

#include <WebView2.h>
#include "Windows/HideWindowsPlatformTypes.h"

#if defined(PlaySound)
#undef PlaySound
#endif
#if defined(UpdateResource)
#undef UpdateResource
#endif
#if defined(max)
#undef max
#endif
#if defined(min)
#undef min
#endif

#include <string>

namespace
{
	using Microsoft::WRL::Callback;
	using Microsoft::WRL::ComPtr;

	static const wchar_t* GT66WebView2HostClassName = L"T66WebView2HostWindow";

	static FString DescribeWindowStyles(DWORD Style, DWORD ExStyle)
	{
		TArray<FString> Bits;
		auto AddIf = [&](bool b, const TCHAR* Name) { if (b) Bits.Add(Name); };

		AddIf((Style & WS_CHILD) != 0, TEXT("WS_CHILD"));
		AddIf((Style & WS_POPUP) != 0, TEXT("WS_POPUP"));
		AddIf((Style & WS_VISIBLE) != 0, TEXT("WS_VISIBLE"));
		AddIf((Style & WS_CLIPSIBLINGS) != 0, TEXT("WS_CLIPSIBLINGS"));
		AddIf((Style & WS_CLIPCHILDREN) != 0, TEXT("WS_CLIPCHILDREN"));

		AddIf((ExStyle & WS_EX_LAYERED) != 0, TEXT("WS_EX_LAYERED"));
		AddIf((ExStyle & WS_EX_TRANSPARENT) != 0, TEXT("WS_EX_TRANSPARENT"));
		AddIf((ExStyle & WS_EX_TOOLWINDOW) != 0, TEXT("WS_EX_TOOLWINDOW"));
		AddIf((ExStyle & WS_EX_TOPMOST) != 0, TEXT("WS_EX_TOPMOST"));
#if defined(WS_EX_NOREDIRECTIONBITMAP)
		AddIf((ExStyle & WS_EX_NOREDIRECTIONBITMAP) != 0, TEXT("WS_EX_NOREDIRECTIONBITMAP"));
#endif

		return FString::Printf(TEXT("style=0x%08x ex=0x%08x [%s]"), Style, ExStyle, *FString::Join(Bits, TEXT("|")));
	}

	static UINT GetDpiForWindowSafe(HWND Hwnd)
	{
		using GetDpiForWindowFn = UINT(WINAPI*)(HWND);
		static GetDpiForWindowFn Fn = nullptr;
		static bool bTried = false;
		if (!bTried)
		{
			bTried = true;
			if (HMODULE User32 = GetModuleHandleW(L"user32.dll"))
			{
#if defined(_MSC_VER)
				#pragma warning(push)
				#pragma warning(disable : 4191) // unsafe conversion from FARPROC to function pointer
#endif
				Fn = reinterpret_cast<GetDpiForWindowFn>(GetProcAddress(User32, "GetDpiForWindow"));
#if defined(_MSC_VER)
				#pragma warning(pop)
#endif
			}
		}
		return Fn ? Fn(Hwnd) : 96u;
	}

	static void LogHwndInfo(const TCHAR* Label, HWND Hwnd)
	{
		if (!Hwnd)
		{
			UE_LOG(LogTemp, Log, TEXT("[TIKTOK][WEBVIEW2] %s hwnd=null"), Label);
			return;
		}

		const LONG_PTR Style = GetWindowLongPtrW(Hwnd, GWL_STYLE);
		const LONG_PTR ExStyle = GetWindowLongPtrW(Hwnd, GWL_EXSTYLE);

		RECT W{};
		GetWindowRect(Hwnd, &W);
		RECT C{};
		GetClientRect(Hwnd, &C);

		HWND Parent = GetParent(Hwnd);
		HWND Owner = GetWindow(Hwnd, GW_OWNER);
		HWND RootOwner = GetAncestor(Hwnd, GA_ROOTOWNER);
		const UINT Dpi = GetDpiForWindowSafe(Hwnd);

		UE_LOG(
			LogTemp,
			Log,
			TEXT("[TIKTOK][WEBVIEW2] %s hwnd=0x%p parent=0x%p owner=0x%p rootOwner=0x%p visible=%d dpi=%u %s winRect=[%d,%d %dx%d] client=[%dx%d]"),
			Label,
			Hwnd,
			Parent,
			Owner,
			RootOwner,
			IsWindowVisible(Hwnd) ? 1 : 0,
			Dpi,
			*DescribeWindowStyles(static_cast<DWORD>(Style), static_cast<DWORD>(ExStyle)),
			static_cast<int32>(W.left),
			static_cast<int32>(W.top),
			static_cast<int32>(W.right - W.left),
			static_cast<int32>(W.bottom - W.top),
			static_cast<int32>(C.right - C.left),
			static_cast<int32>(C.bottom - C.top));
	}

	static FString ShortUrl(const FString& Url)
	{
		if (Url.Len() <= 180) return Url;
		return Url.Left(180) + TEXT("...");
	}

	static HWND CreateHostOwnedPopup(HWND Owner)
	{
		WNDCLASSW wc{};
		wc.lpfnWndProc = DefWindowProcW;
		wc.hInstance = GetModuleHandleW(nullptr);
		wc.lpszClassName = GT66WebView2HostClassName;
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		RegisterClassW(&wc);

		// NOTE: A WS_CHILD hosted on top of UE's flip-model swapchain can create "punch-through" / desktop bleed.
		// Use an owned popup instead (no taskbar icon, stays above the game window).
		// Do NOT steal activation/focus. Also make the window transparent to mouse hit-testing so the player can keep playing.
		const DWORD ExStyle = WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT;
		const DWORD Style = WS_POPUP;
		HWND Hwnd = CreateWindowExW(
			ExStyle,
			GT66WebView2HostClassName,
			L"",
			Style,
			0, 0, 1, 1,
			Owner, // owner window (NOT a child)
			nullptr,
			GetModuleHandleW(nullptr),
			nullptr);

		LogHwndInfo(TEXT("HostCreated"), Hwnd);
		return Hwnd;
	}

	static std::wstring ToWide(const FString& S) { return std::wstring(*S); }
}

struct FT66WebView2Host::FImpl
{
	using CreateEnvironmentWithOptionsFn = HRESULT(STDAPICALLTYPE*)(
		PCWSTR BrowserExecutableFolder,
		PCWSTR UserDataFolder,
		ICoreWebView2EnvironmentOptions* EnvironmentOptions,
		ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* EnvironmentCreatedHandler);

	HMODULE LoaderModule = nullptr;
	CreateEnvironmentWithOptionsFn CreateEnv = nullptr;

	HWND Parent = nullptr; // Owner window (UE game window)
	HWND Host = nullptr;   // Our owned popup

	ComPtr<ICoreWebView2Environment> Env;
	ComPtr<ICoreWebView2Controller> Ctrl;
	ComPtr<ICoreWebView2> View;

	EventRegistrationToken NavToken{};
	EventRegistrationToken PopupToken{};
	EventRegistrationToken NavCompletedToken{};

	bool bHasPendingRect = false;
	FIntRect PendingRect;
	FString PendingUrl;
	bool bWantVisible = false;
	double DesiredZoomFactor = 1.0;
	bool bHasEverNavigated = false;

	void SetWebViewMuted(bool bMuted)
	{
		// Hard requirement: no audio may leak when the overlay is hidden.
		// WebView2 supports global document mute via ICoreWebView2_8.
		if (!View) return;
		ComPtr<ICoreWebView2_8> View8;
		if (SUCCEEDED(View.As(&View8)) && View8)
		{
			View8->put_IsMuted(bMuted ? 1 : 0);
		}
	}

	~FImpl()
	{
		if (View && NavToken.value) { View->remove_NavigationStarting(NavToken); }
		if (View && PopupToken.value) { View->remove_NewWindowRequested(PopupToken); }
		if (View && NavCompletedToken.value) { View->remove_NavigationCompleted(NavCompletedToken); }
		NavToken.value = 0;
		PopupToken.value = 0;
		NavCompletedToken.value = 0;

		Ctrl.Reset();
		View.Reset();
		Env.Reset();

		if (Host) { DestroyWindow(Host); Host = nullptr; }
		if (LoaderModule) { FreeLibrary(LoaderModule); LoaderModule = nullptr; }
	}

	bool LoadLoader()
	{
		if (CreateEnv) return true;

		LoaderModule = LoadLibraryW(L"WebView2Loader.dll");
		if (!LoaderModule)
		{
			const FString Fallback = FPaths::ConvertRelativePathToFull(
				FPaths::Combine(FPaths::ProjectDir(), TEXT("ThirdParty"), TEXT("WebView2"), TEXT("bin"), TEXT("Win64"), TEXT("WebView2Loader.dll")));
			LoaderModule = LoadLibraryW(*Fallback);
			if (!LoaderModule)
			{
				UE_LOG(LogTemp, Error, TEXT("[TIKTOK][WEBVIEW2] Missing WebView2Loader.dll. Expected next to exe or at '%s'."), *Fallback);
				return false;
			}
			UE_LOG(LogTemp, Log, TEXT("[TIKTOK][WEBVIEW2] Loaded WebView2Loader.dll from '%s'."), *Fallback);
		}

	#if defined(_MSC_VER)
		#pragma warning(push)
		#pragma warning(disable : 4191) // unsafe conversion from FARPROC to function pointer
	#endif
		CreateEnv = reinterpret_cast<CreateEnvironmentWithOptionsFn>(
			GetProcAddress(LoaderModule, "CreateCoreWebView2EnvironmentWithOptions"));
	#if defined(_MSC_VER)
		#pragma warning(pop)
	#endif
		if (!CreateEnv)
		{
			UE_LOG(LogTemp, Error, TEXT("[TIKTOK][WEBVIEW2] WebView2Loader.dll missing CreateCoreWebView2EnvironmentWithOptions."));
			return false;
		}
		return true;
	}

	bool IsAllowedUrl(const FString& Url) const
	{
		FString U = Url.TrimStartAndEnd();

		// Strict scheme allowlist: only allow http(s) for network content.
		// Allow about:blank as an internal navigation.
		const int32 SepIdx = U.Find(TEXT("://"), ESearchCase::IgnoreCase);
		if (SepIdx == INDEX_NONE)
		{
			const FString Lower = U.ToLower();
			return Lower == TEXT("about:blank");
		}

		const FString Scheme = U.Left(SepIdx).ToLower();
		if (Scheme != TEXT("https") && Scheme != TEXT("http"))
		{
			return false;
		}

		U = U.Mid(SepIdx + 3);
		int32 EndIdx = U.Len();
		const int32 SlashIdx = U.Find(TEXT("/"));
		const int32 QIdx = U.Find(TEXT("?"));
		const int32 HashIdx = U.Find(TEXT("#"));
		if (SlashIdx != INDEX_NONE) EndIdx = FMath::Min(EndIdx, SlashIdx);
		if (QIdx != INDEX_NONE) EndIdx = FMath::Min(EndIdx, QIdx);
		if (HashIdx != INDEX_NONE) EndIdx = FMath::Min(EndIdx, HashIdx);
		FString HostPort = U.Left(EndIdx);
		int32 ColonIdx = INDEX_NONE;
		if (HostPort.FindChar(TEXT(':'), ColonIdx)) { HostPort = HostPort.Left(ColonIdx); }
		HostPort.ToLowerInline();
		if (HostPort.IsEmpty()) return false;

		auto IsExactOrSubdomainOf = [&](const TCHAR* Root) -> bool
		{
			const FString R(Root);
			return HostPort == R || HostPort.EndsWith(FString(TEXT(".")) + R);
		};

		return
			IsExactOrSubdomainOf(TEXT("tiktok.com")) ||
			IsExactOrSubdomainOf(TEXT("tiktokcdn.com")) ||
			IsExactOrSubdomainOf(TEXT("tiktokcdn-us.com")) ||
			IsExactOrSubdomainOf(TEXT("tiktokcdn-eu.com")) ||
			IsExactOrSubdomainOf(TEXT("tiktokv.com")) ||
			IsExactOrSubdomainOf(TEXT("ttwstatic.com")) ||
			IsExactOrSubdomainOf(TEXT("ibytedtos.com")) ||
			IsExactOrSubdomainOf(TEXT("ibyteimg.com")) ||
			IsExactOrSubdomainOf(TEXT("byteoversea.com")) ||
			HostPort == TEXT("storage.googleapis.com");
	}

	void ApplyBoundsScreen(const FIntRect& ScreenR)
	{
		if (Host)
		{
			MoveWindow(Host, ScreenR.Min.X, ScreenR.Min.Y, ScreenR.Width(), ScreenR.Height(), 1);
		}
		if (Ctrl)
		{
			RECT Rc{};
			// Controller bounds are in *Host client* coordinates.
			Rc.left = 0;
			Rc.top = 0;
			Rc.right = ScreenR.Width();
			Rc.bottom = ScreenR.Height();
			Ctrl->put_Bounds(Rc);

			// Make TikTok fit better in the phone panel. (Best-effort; TikTok layout varies.)
			// Shrink slightly for narrow panels so UI/video doesn't get clipped.
			static constexpr double DesignWidthPx = 520.0;
			const double Target = FMath::Clamp(static_cast<double>(ScreenR.Width()) / DesignWidthPx, 0.65, 1.0);
			if (FMath::Abs(Target - DesiredZoomFactor) > 0.001)
			{
				DesiredZoomFactor = Target;
				Ctrl->put_ZoomFactor(DesiredZoomFactor);
				UE_LOG(LogTemp, Log, TEXT("[TIKTOK][WEBVIEW2] ZoomFactor=%.3f (w=%d, design=%.0f)"), DesiredZoomFactor, ScreenR.Width(), DesignWidthPx);
			}

			Ctrl->NotifyParentWindowPositionChanged();
		}
	}

	bool EnsureCreated(HWND InParent, const FString& UserDataFolder)
	{
		if (View) return true;
		if (!LoadLoader()) return false;

		Parent = InParent;
		if (!Parent)
		{
			UE_LOG(LogTemp, Error, TEXT("[TIKTOK][WEBVIEW2] No parent HWND."));
			return false;
		}
		LogHwndInfo(TEXT("Owner"), Parent);

		if (!Host)
		{
			Host = CreateHostOwnedPopup(Parent);
			if (!Host)
			{
				UE_LOG(LogTemp, Error, TEXT("[TIKTOK][WEBVIEW2] Failed to create host window."));
				return false;
			}
		}

		std::wstring UserDataWide = ToWide(UserDataFolder);
		UE_LOG(LogTemp, Log, TEXT("[TIKTOK][WEBVIEW2] Creating environment. UserData='%s'"), *UserDataFolder);

		HRESULT Hr = CreateEnv(
			nullptr,
			UserDataWide.c_str(),
			nullptr,
			Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
				[this](HRESULT Result, ICoreWebView2Environment* InEnv) -> HRESULT
				{
					if (FAILED(Result) || !InEnv)
					{
						UE_LOG(LogTemp, Error, TEXT("[TIKTOK][WEBVIEW2] Environment create failed (0x%08x)."), static_cast<uint32>(Result));
						return S_OK;
					}

					Env = InEnv;
					UE_LOG(LogTemp, Log, TEXT("[TIKTOK][WEBVIEW2] Environment ready."));
						LogHwndInfo(TEXT("HostPreController"), Host);

					Env->CreateCoreWebView2Controller(
						Host,
						Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
							[this](HRESULT Result2, ICoreWebView2Controller* InCtrl) -> HRESULT
							{
								if (FAILED(Result2) || !InCtrl)
								{
									UE_LOG(LogTemp, Error, TEXT("[TIKTOK][WEBVIEW2] Controller create failed (0x%08x)."), static_cast<uint32>(Result2));
									return S_OK;
								}

								Ctrl = InCtrl;
								Ctrl->get_CoreWebView2(&View);

								// IMPORTANT: This is toggled on open/close. If we hide it, we must re-show it.
								Ctrl->put_IsVisible(bWantVisible ? 1 : 0);
								UE_LOG(LogTemp, Log, TEXT("[TIKTOK][WEBVIEW2] Controller/WebView ready."));
								LogHwndInfo(TEXT("HostPostController"), Host);

								if (bHasPendingRect)
								{
									ApplyBoundsScreen(PendingRect);
								}
								if (Host)
								{
									ShowWindow(Host, bWantVisible ? SW_SHOWNA : SW_HIDE);
								}

								ComPtr<ICoreWebView2Settings> Settings;
								if (SUCCEEDED(View->get_Settings(&Settings)) && Settings)
								{
									Settings->put_AreDefaultContextMenusEnabled(0);
									Settings->put_AreDevToolsEnabled(0);
									Settings->put_IsStatusBarEnabled(0);
									Settings->put_IsZoomControlEnabled(0);
									Settings->put_IsBuiltInErrorPageEnabled(1);
								}

								// Force an opaque background to avoid any transparency / desktop bleed artifacts.
								ComPtr<ICoreWebView2Controller2> Ctrl2;
								if (SUCCEEDED(Ctrl.As(&Ctrl2)) && Ctrl2)
								{
									COREWEBVIEW2_COLOR C{};
									C.A = 255;
									C.R = 0;
									C.G = 0;
									C.B = 0;
									Ctrl2->put_DefaultBackgroundColor(C);
								}

								View->add_NewWindowRequested(
									Callback<ICoreWebView2NewWindowRequestedEventHandler>(
										[this](ICoreWebView2* /*sender*/, ICoreWebView2NewWindowRequestedEventArgs* Args) -> HRESULT
										{
											if (Args)
											{
												Args->put_Handled(1);
											}
											return S_OK;
										}).Get(),
									&PopupToken);

								View->add_NavigationStarting(
									Callback<ICoreWebView2NavigationStartingEventHandler>(
										[this](ICoreWebView2* /*sender*/, ICoreWebView2NavigationStartingEventArgs* Args) -> HRESULT
										{
											if (!Args) return S_OK;
											LPWSTR Raw = nullptr;
											if (SUCCEEDED(Args->get_Uri(&Raw)) && Raw)
											{
												const FString Url(Raw);
												CoTaskMemFree(Raw);
												if (!IsAllowedUrl(Url))
												{
													UE_LOG(LogTemp, Warning, TEXT("[TIKTOK][WEBVIEW2] BLOCK nav '%s'"), *ShortUrl(Url));
													Args->put_Cancel(1);
													return S_OK;
												}
												UE_LOG(LogTemp, Log, TEXT("[TIKTOK][WEBVIEW2] NavStarting '%s'"), *ShortUrl(Url));
											}
											return S_OK;
										}).Get(),
									&NavToken);

								// QR visibility tweaks (run after page nav completes).
								View->add_NavigationCompleted(
									Callback<ICoreWebView2NavigationCompletedEventHandler>(
										[this](ICoreWebView2* /*sender*/, ICoreWebView2NavigationCompletedEventArgs* /*Args*/) -> HRESULT
										{
											if (!View) return S_OK;

											LPWSTR Raw = nullptr;
											FString Url;
											if (SUCCEEDED(View->get_Source(&Raw)) && Raw)
											{
												Url = FString(Raw);
												CoTaskMemFree(Raw);
											}

											UE_LOG(LogTemp, Log, TEXT("[TIKTOK][WEBVIEW2] NavCompleted '%s'"), *ShortUrl(Url));

											// Best-effort: make TikTok "video-only" by hiding common chrome (nav/header/footer/sidebar).
											// Do NOT apply this on the QR login screen.
											if (!Url.Contains(TEXT("login/qrcode"), ESearchCase::IgnoreCase))
											{
												static const wchar_t* VideoOnlyScript = LR"JS(
													(function(){
														try{
															var href = (window.location && window.location.href) ? window.location.href : '';
															if (href.toLowerCase().indexOf('tiktok.com') === -1) return;

															function apply(){
																try{
																	var id='t66_video_only_css';
																	var st=document.getElementById(id);
																	if(!st){
																		st=document.createElement('style');
																		st.id=id;
																		st.type='text/css';
																		(document.head||document.documentElement).appendChild(st);
																	}
																	st.textContent =
																		'html,body{background:#000 !important;margin:0 !important;padding:0 !important;overflow:hidden !important;}' +
																		'*{scrollbar-width:none !important;}' +
																		'*::-webkit-scrollbar{width:0 !important;height:0 !important;}' +
																		'header,nav,footer,aside,[role=\"banner\"],[role=\"navigation\"],[role=\"contentinfo\"]{display:none !important;}' +
																		'[data-e2e*=\"nav\" i],[data-e2e*=\"header\" i],[data-e2e*=\"footer\" i],[data-e2e*=\"sidebar\" i]{display:none !important;}' +
																		'[class*=\"SideNav\" i],[class*=\"Navbar\" i],[class*=\"Navigation\" i],[class*=\"TopNav\" i],[class*=\"BottomNav\" i]{display:none !important;}' +
																		'main,[role=\"main\"]{width:100vw !important;max-width:100vw !important;margin:0 !important;padding:0 !important;}' +
																		// Critical: stop TikTok from cropping the video to fill; show full video with letterboxing.
																		'video{background:#000 !important;object-fit:contain !important;}' +
																		// Many TikTok layouts wrap the video in an element that controls sizing; make it respect viewport.
																		'[class*=\"Video\" i],[class*=\"Player\" i]{max-width:100vw !important;max-height:100vh !important;}';
																}catch(e){}
															}

															apply();
															setTimeout(apply, 250);
															setTimeout(apply, 900);
														}catch(e){}
													})();
												)JS";

												ExecuteScriptNoResult(VideoOnlyScript, TEXT("VideoOnlyCSS"));
											}

											if (!Url.Contains(TEXT("login/qrcode"), ESearchCase::IgnoreCase))
											{
												return S_OK;
											}

											// Apply zoom out + hide scrollbars + try to ensure QR is fully visible.
											static const wchar_t* QrScript = LR"JS(
												(function(){
													try {
														var href = (window.location && window.location.href) ? window.location.href : '';
														if (href.toLowerCase().indexOf('login/qrcode') === -1) return;

														var z = 0.85;
														if (document.documentElement) document.documentElement.style.zoom = z;
														if (document.body) document.body.style.zoom = z;

														if (!document.getElementById('t66_hide_scrollbars')) {
															var st = document.createElement('style');
															st.id = 't66_hide_scrollbars';
															st.textContent =
																'*{scrollbar-width:none !important;}' +
																'*::-webkit-scrollbar{width:0 !important;height:0 !important;}' +
																'html,body{overflow-x:hidden !important;}';
															(document.head || document.documentElement).appendChild(st);
														}

														function getRootScrollEl() {
															return document.scrollingElement || document.documentElement || document.body;
														}

														function findQrEl() {
															var sel = ['img[src*=\"qrcode\"]','img[alt*=\"QR\"]','img[alt*=\"qr\"]','canvas'];
															for (var i=0;i<sel.length;i++){
																var el = document.querySelector(sel[i]);
																if (el) return el;
															}
															var cands = [];
															var imgs = Array.prototype.slice.call(document.images || []);
															for (var j=0;j<imgs.length;j++){
																var im = imgs[j];
																var w = im.naturalWidth || 0, h = im.naturalHeight || 0;
																if (w >= 160 && h >= 160) cands.push(im);
															}
															var canv = Array.prototype.slice.call(document.querySelectorAll('canvas') || []);
															for (var k=0;k<canv.length;k++){
																var cv = canv[k];
																var cw = cv.width || 0, ch = cv.height || 0;
																if (cw >= 160 && ch >= 160) cands.push(cv);
															}
															var best=null, bestArea=0;
															for (var n=0;n<cands.length;n++){
																var e=cands[n];
																var r=e.getBoundingClientRect();
																if (!r || r.width<=0 || r.height<=0) continue;
																var area=r.width*r.height;
																if (area>bestArea){bestArea=area; best=e;}
															}
															return best;
														}

														function ensureQrVisible(){
															try{
																var root=getRootScrollEl();
																var qr=findQrEl();
																if (!root || !qr) return;
																if (typeof qr.scrollIntoView==='function') qr.scrollIntoView({block:'center', inline:'nearest'});
																var r=qr.getBoundingClientRect();
																var vw=window.innerWidth || document.documentElement.clientWidth || 0;
																if (vw<=0) return;
																var pad=8;
																var dx=0;
																if (r.right>vw-pad) dx=(r.right-(vw-pad));
																else if (r.left<pad) dx=(r.left-pad);
																if (dx!==0) root.scrollLeft=Math.max(0,(root.scrollLeft||0)+dx);
															}catch(e){}
														}

														ensureQrVisible();
														setTimeout(ensureQrVisible,150);
														setTimeout(ensureQrVisible,500);
														setTimeout(ensureQrVisible,1000);
													} catch(e) {}
												})();
											)JS";

											View->ExecuteScript(QrScript, nullptr);
											return S_OK;
										}).Get(),
									&NavCompletedToken);

								if (!PendingUrl.IsEmpty())
								{
									const FString Url = PendingUrl;
									PendingUrl.Reset();
									Navigate(Url);
								}

								return S_OK;
							}).Get());

					return S_OK;
				}).Get());

		if (FAILED(Hr))
		{
			UE_LOG(LogTemp, Error, TEXT("[TIKTOK][WEBVIEW2] CreateEnvironmentWithOptions failed (0x%08x)."), static_cast<uint32>(Hr));
			return false;
		}

		return true;
	}

	void Navigate(const FString& Url)
	{
		if (!View)
		{
			PendingUrl = Url;
			return;
		}
		if (!IsAllowedUrl(Url))
		{
			UE_LOG(LogTemp, Warning, TEXT("[TIKTOK][WEBVIEW2] Refusing Navigate '%s'"), *ShortUrl(Url));
			return;
		}
		bHasEverNavigated = true;
		std::wstring Wide = ToWide(Url);
		View->Navigate(Wide.c_str());
		UE_LOG(LogTemp, Log, TEXT("[TIKTOK][WEBVIEW2] Navigate '%s'"), *ShortUrl(Url));
	}

	void ExecuteScriptNoResult(const TCHAR* Script, const TCHAR* DebugTag)
	{
		if (!View) return;
		View->ExecuteScript(Script, nullptr);
		if (DebugTag)
		{
			UE_LOG(LogTemp, Log, TEXT("[TIKTOK][WEBVIEW2] ExecuteScript '%s'"), DebugTag);
		}
	}

	void MoveVideo(int32 Dir)
	{
		Dir = (Dir >= 0) ? 1 : -1;

		// IMPORTANT: keep this script small enough for MSVC string literal limits.
		// Also: avoid expensive DOM scans; rely on elementsFromPoint at viewport center when possible.
		FString Script;
		Script.Reserve(8192);

		Script += TEXT("(function(){try{");
		Script += TEXT("var TT=window.__t66TikTok; if(!TT){TT=window.__t66TikTok={q:[],busy:false,sc:null,hidden:false,__moveId:0};}");
		Script += TEXT("TT.hidden = false;"); // user pressed Q/E; if we are visible, allow playback/moves

		Script += TEXT("function raf(){return new Promise(function(r){requestAnimationFrame(function(){r();});});}");
		Script += TEXT("function sl(ms){return new Promise(function(r){setTimeout(r,ms);});}");
		Script += TEXT("function now(){return (window.performance&&performance.now)?performance.now():Date.now();}");

		Script += TEXT("function area(r){var w=Math.max(0,Math.min(r.right,innerWidth)-Math.max(r.left,0));var h=Math.max(0,Math.min(r.bottom,innerHeight)-Math.max(r.top,0));return w*h;}");

		Script += TEXT("function centerV(){");
		Script += TEXT("var cx=(innerWidth||0)*0.5,cy=(innerHeight||0)*0.5;");
		Script += TEXT("var els=(document.elementsFromPoint?document.elementsFromPoint(cx,cy):[])||[];");
		Script += TEXT("for(var i=0;i<els.length&&i<20;i++){var e=els[i];if(!e)continue;if(e.tagName==='VIDEO')return e;if(e.querySelector){var v=e.querySelector('video');if(v)return v;}}");
		Script += TEXT("var vids=document.querySelectorAll('video');if(!vids||!vids.length)return null;");
		Script += TEXT("var best=null,ba=0;for(var j=0;j<vids.length&&j<20;j++){var vv=vids[j];if(!vv)continue;var rr=vv.getBoundingClientRect();if(!rr)continue;var a=area(rr);if(a>ba){ba=a;best=vv;}}return best;}");

		Script += TEXT("function isSc(el){try{var cs=getComputedStyle(el);var oy=cs?cs.overflowY:'';return (oy==='auto'||oy==='scroll'||oy==='overlay')&&el.scrollHeight>el.clientHeight+4;}catch(e){return false;}}");
		Script += TEXT("function findSc(el){var p=el;while(p&&p!==document.body&&p!==document.documentElement){if(isSc(p))return p;p=p.parentElement;}return document.scrollingElement||document.documentElement||document.body;}");
		Script += TEXT("function sc(){if(TT.sc&&TT.sc.isConnected)return TT.sc;var v=centerV();TT.sc=findSc(v?v.parentElement:document.body);return TT.sc;}");

		Script += TEXT("function pos(s){try{if(s===document.body||s===document.documentElement||s===document.scrollingElement)return (window.scrollY||0);return (s&&typeof s.scrollTop==='number')?s.scrollTop:0;}catch(e){return 0;}}");
		Script += TEXT("async function idle(s,ms){var t0=now();var last=pos(s);var lastCh=now();");
		Script += TEXT("while(now()-t0<ms){await raf();var cur=pos(s);");
		Script += TEXT("if(Math.abs(cur-last)>1){last=cur;lastCh=now();}else if(now()-lastCh>120)break;}}");

		Script += TEXT("async function stable(ms){var t0=now();var last=null,n=0;");
		Script += TEXT("while(now()-t0<ms){await raf();var v=centerV();if(v&&v===last){n++;if(n>=6)return v;}else{last=v;n=v?1:0;}}return last;}");

		Script += TEXT("function inView(r){if(!r)return false;return !(r.bottom<0||r.top>innerHeight||r.right<0||r.left>innerWidth);}");
		Script += TEXT("function nearVideos(limit){var vids=document.querySelectorAll('video');var out=[];if(!vids)return out;for(var i=0;i<vids.length&&i<limit;i++){var v=vids[i];if(!v)continue;var r=null;try{r=v.getBoundingClientRect();}catch(e){}");
		Script += TEXT("if(!r||r.height<80||r.width<80)continue;if(!inView(r))continue;out.push(v);}return out;}");
		Script += TEXT("function pickNeighbor(cur,dir){var vids=document.querySelectorAll('video');if(!vids||!vids.length)return null;");
		Script += TEXT("var cr=null,cc=(innerHeight||800)*0.5;try{if(cur){cr=cur.getBoundingClientRect();if(cr)cc=cr.top+cr.height*0.5;}}catch(e){}");
		Script += TEXT("var best=null,bestC=(dir>0)?1e18:-1e18;for(var i=0;i<vids.length&&i<80;i++){var v=vids[i];if(!v||v===cur)continue;");
		Script += TEXT("var r=null;try{r=v.getBoundingClientRect();}catch(e){}if(!r||r.height<80||r.width<80)continue;");
		Script += TEXT("var c=r.top+r.height*0.5;if(c<-innerHeight*0.5||c>innerHeight*1.5)continue;");
		Script += TEXT("if(dir>0){if(c>cc+10&&c<bestC){bestC=c;best=v;}}else{if(c<cc-10&&c>bestC){bestC=c;best=v;}}}return best;}");
		Script += TEXT("function snapTo(v,s){if(!v)return false;try{var r=v.getBoundingClientRect();if(!r)return false;var cy=(innerHeight||800)*0.5;var d=(r.top+r.height*0.5)-cy;if(Math.abs(d)<2)return true;");
		Script += TEXT("if(s===document.body||s===document.documentElement||s===document.scrollingElement){window.scrollBy({top:d,left:0,behavior:'auto'});}else{s.scrollTop=(s.scrollTop||0)+d;}return true;}catch(e){return false;}}");
		Script += TEXT("function scrollToCenter(v){try{if(!v||!v.scrollIntoView)return false;v.scrollIntoView({block:'center',inline:'nearest',behavior:'smooth'});return true;}catch(e){return false;}}");

		// While the viewer is visible:
		// - enforce exactly one active video (center-most) playing + unmuted
		// - immediately stop (mute+pause) any other visible videos so old audio never bleeds
		// - block TikTok from pausing videos via prototype hook (except when we explicitly allow internal pause)
		Script += TEXT("if(!TT._init){TT._init=1;TT.active=null;TT._timer=0;TT.moving=0;TT.__allow=0;");
		Script += TEXT("try{TT._origPause=HTMLMediaElement.prototype.pause;}catch(e){}");
		Script += TEXT("try{HTMLMediaElement.prototype.pause=function(){try{var T=window.__t66TikTok;if(!T||!T._origPause)return; if(T.hidden||T.__allow) return T._origPause.apply(this,arguments); return;}catch(e){try{var T2=window.__t66TikTok;if(T2&&T2._origPause) return T2._origPause.apply(this,arguments);}catch(e2){}}};}catch(e){}");
		Script += TEXT("try{var d=Object.getOwnPropertyDescriptor(HTMLMediaElement.prototype,'muted');");
		Script += TEXT("if(d&&d.set&&d.get&&d.configurable){TT._origMutedSet=d.set;TT._origMutedGet=d.get;");
		Script += TEXT("Object.defineProperty(HTMLMediaElement.prototype,'muted',{configurable:true,enumerable:d.enumerable,");
		Script += TEXT("get:function(){return TT._origMutedGet.call(this);},");
		Script += TEXT("set:function(v){var T=window.__t66TikTok;if(!T||!T._origMutedSet)return; if(T.hidden||T.__allow) return T._origMutedSet.call(this,!!v); if(this===T.active) return T._origMutedSet.call(this,false); return T._origMutedSet.call(this,true);}});}}catch(e){}");
		Script += TEXT("}");

		Script += TEXT("function setMuted(v,b){try{v.muted=!!b;v.defaultMuted=!!b;if(b){v.setAttribute&&v.setAttribute('muted','');}else{v.removeAttribute&&v.removeAttribute('muted');}}catch(e){}}");
		Script += TEXT("function safePlay(v){if(!v) return; try{v.volume=1;v.playbackRate=1;}catch(e){} setMuted(v,false); try{if(v.paused){var p=v.play&&v.play();if(p&&p.catch)p.catch(function(){});} }catch(e){} }");
		Script += TEXT("function safeStop(v){if(!v) return; setMuted(v,true); try{if(TT._origPause){TT.__allow=1;TT._origPause.call(v);TT.__allow=0;}else{v.pause&&v.pause();}}catch(e){} TT.__allow=0; }");

		Script += TEXT("function enforce(){if(TT.hidden||TT.moving) return; var v=TT.active; if(!v||!v.isConnected) v=centerV(); if(!v) return; TT.active=v;");
		Script += TEXT("var vs=nearVideos(70); for(var i=0;i<vs.length;i++){var vv=vs[i]; if(vv===v) continue; safeStop(vv);} safePlay(v); }");

		Script += TEXT("TT.start=TT.start||function(){TT.hidden=false; if(TT._timer) return; TT._timer=setInterval(enforce,200); setTimeout(enforce,0);};");
		Script += TEXT("TT.stop=TT.stop||function(){TT.hidden=true; if(TT._timer){clearInterval(TT._timer);TT._timer=0;}};");

		Script += TEXT("async function moveOnce(dir){TT.start(); TT.moving=1;");
		Script += TEXT("var before=TT.active||centerV(); var t=pickNeighbor(before,dir)||pickNeighbor(centerV(),dir); var s=t?findSc(t.parentElement||t):sc();");
		Script += TEXT("if(before) safeStop(before);");
		Script += TEXT("var dy=Math.floor((innerHeight||800)*1.10)*(dir>0?1:-1);");
		Script += TEXT("if(t){try{t.scrollIntoView&&t.scrollIntoView({block:'center',inline:'nearest',behavior:'auto'});}catch(e){} snapTo(t,s);}else{try{if(s&&s.scrollBy){s.scrollBy({top:dy,left:0,behavior:'auto'});}else{window.scrollBy({top:dy,left:0,behavior:'auto'});}}catch(e){}}");
		Script += TEXT("await idle(s,1200); var v=centerV();");
		Script += TEXT("if(before && v===before){"); // still on same video -> force a bigger commit step
		Script += TEXT("try{if(t&&t.isConnected){t.scrollIntoView&&t.scrollIntoView({block:'center',inline:'nearest',behavior:'auto'}); snapTo(t,s);}else{if(s&&s.scrollBy){s.scrollBy({top:dy,left:0,behavior:'auto'});}else{window.scrollBy({top:dy,left:0,behavior:'auto'});}}}catch(e){}");
		Script += TEXT("await idle(s,900); v=centerV(); if(v===before && t && t.isConnected) v=t; }");
		Script += TEXT("if(v){TT.active=v; snapTo(v,s);} TT.moving=0; enforce(); }");

		Script += TEXT("TT.move=TT.move||function(dir){TT.q.push(dir); if(TT.busy) return; TT.busy=true; (async function(){");
		Script += TEXT("while(TT.q.length){var d=TT.q.shift(); await moveOnce(d); await sl(80);} TT.busy=false;})();};");

		Script += FString::Printf(TEXT("TT.move(%d);"), Dir);
		Script += TEXT("}catch(e){}})();");

		ExecuteScriptNoResult(*Script, Dir > 0 ? TEXT("NextVideo") : TEXT("PrevVideo"));
	}

	void PrevVideo()
	{
		MoveVideo(-1);
	}

	void NextVideo()
	{
		MoveVideo(+1);
	}
};

FT66WebView2Host::FT66WebView2Host()
{
	Impl = MakeUnique<FImpl>();
}

FT66WebView2Host::~FT66WebView2Host() = default;

bool FT66WebView2Host::EnsureCreated(void* ParentWindowHandle, const FString& UserDataFolder)
{
	return Impl ? Impl->EnsureCreated(static_cast<HWND>(ParentWindowHandle), UserDataFolder) : false;
}

void FT66WebView2Host::ShowAtRect(const FIntRect& ClientRectPx)
{
	// Deprecated path (we no longer host as a child window). Treat it as a screen rect for safety.
	ShowAtScreenRect(ClientRectPx);
}

void FT66WebView2Host::ShowAtScreenRect(const FIntRect& ScreenRectPx)
{
	if (!Impl) return;
	Impl->PendingRect = ScreenRectPx;
	Impl->bHasPendingRect = true;
	Impl->bWantVisible = true;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[TIKTOK][WEBVIEW2] ShowAtScreenRect [%d,%d -> %d,%d] (%dx%d)"),
		ScreenRectPx.Min.X,
		ScreenRectPx.Min.Y,
		ScreenRectPx.Max.X,
		ScreenRectPx.Max.Y,
		ScreenRectPx.Width(),
		ScreenRectPx.Height());

	// Only show after WebView2 controller is ready; otherwise this can appear as a transparent/blank overlay.
	if (Impl->Ctrl && Impl->Host)
	{
		// Unmute at WebView2 layer (Hide() hard-mutes here).
		Impl->SetWebViewMuted(false);
		Impl->ApplyBoundsScreen(ScreenRectPx);
		Impl->Ctrl->put_IsVisible(1);
		ShowWindow(Impl->Host, SW_SHOWNA);
		SetWindowPos(Impl->Host, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

		// If the user toggled TikTok back on, resume playback of the currently visible video.
		// Harden against autoplay policy: play muted first, then unmute shortly after.
		static const wchar_t* ResumeScript = LR"JS(
			(function(){
				try{
					if (window.__t66TikTok){ window.__t66TikTok.hidden = false; }
					var vids = Array.prototype.slice.call(document.querySelectorAll('video') || []);
					var best=null, bestArea=0;
					for (var i=0;i<vids.length;i++){
						var v=vids[i]; if(!v) continue;
						var r=v.getBoundingClientRect(); if(!r) continue;
						var w=Math.max(0, Math.min(r.right, window.innerWidth)-Math.max(r.left,0));
						var h=Math.max(0, Math.min(r.bottom, window.innerHeight)-Math.max(r.top,0));
						var a=w*h;
						if(a>bestArea){bestArea=a; best=v;}
					}
					if (best){
						try{
							if (typeof best.volume === 'number') best.volume = 1.0;
							try{
								best.muted = false;
								best.defaultMuted = false;
								if (best.removeAttribute) best.removeAttribute('muted');
							}catch(e){}
							if (best.play) best.play().catch(function(){});
							setTimeout(function(){
								try{
									try{
										best.muted = false;
										best.defaultMuted = false;
										if (best.removeAttribute) best.removeAttribute('muted');
									}catch(e){}
									if (best.play) best.play().catch(function(){});
								}catch(e){}
							}, 120);

							// TikTok sometimes auto-pauses shortly after landing; try again once.
							setTimeout(function(){
								try{
									if (best && best.paused && best.play){
										try{
											best.muted = false;
											best.defaultMuted = false;
											if (best.removeAttribute) best.removeAttribute('muted');
										}catch(e){}
										best.play().catch(function(){});
										setTimeout(function(){
											try{
												try{
													best.muted = false;
													best.defaultMuted = false;
													if (best.removeAttribute) best.removeAttribute('muted');
												}catch(e){}
												if (best.play) best.play().catch(function(){});
											}catch(e){}
										}, 120);
									}
								}catch(e){}
							}, 1100);
						}catch(e){}
					}

					// Ensure the TikTok guardian is running while visible (enforces single-audio + force-play).
					try{
						if (window.__t66TikTok && window.__t66TikTok.start) { window.__t66TikTok.start(); }
					}catch(e){}
				}catch(e){}
			})();
		)JS";
		Impl->ExecuteScriptNoResult(ResumeScript, TEXT("ResumeVisibleVideo"));
	}
}

void FT66WebView2Host::Hide()
{
	if (Impl)
	{
		Impl->bWantVisible = false;

		// Pause/mute any playing media so audio never leaks after hiding the overlay.
		// Also hard-mute at the WebView2 layer (unbreakable; protects against any JS failure).
		Impl->SetWebViewMuted(true);
		static const wchar_t* PauseScript = LR"JS(
			(function(){
				try{
					if (window.__t66TikTok){
						window.__t66TikTok.hidden = true;
						window.__t66TikTok.__moveId = (window.__t66TikTok.__moveId||0) + 1; // cancel keepAlive loops
						if (window.__t66TikTok.stop) { window.__t66TikTok.stop(); }
					}
					var els = Array.prototype.slice.call(document.querySelectorAll('video,audio') || []);
					for (var i=0;i<els.length;i++){
						var e=els[i]; if(!e) continue;
						try{ e.muted = true; e.defaultMuted = true; if (e.setAttribute) e.setAttribute('muted',''); }catch(ex){}
						try{ e.pause && e.pause(); }catch(ex2){}
					}
				}catch(e){}
			})();
		)JS";
		Impl->ExecuteScriptNoResult(PauseScript, TEXT("PauseAllOnHide"));

		if (Impl->Ctrl)
		{
			Impl->Ctrl->put_IsVisible(0);
		}
		if (Impl->Host)
		{
			ShowWindow(Impl->Host, SW_HIDE);
		}
	}
}

void FT66WebView2Host::Navigate(const FString& Url)
{
	if (Impl)
	{
		Impl->Navigate(Url);
	}
}

void FT66WebView2Host::PrevVideo()
{
	if (Impl)
	{
		Impl->PrevVideo();
	}
}

void FT66WebView2Host::NextVideo()
{
	if (Impl)
	{
		Impl->NextVideo();
	}
}

bool FT66WebView2Host::HasEverNavigated() const
{
	return Impl && Impl->bHasEverNavigated;
}

bool FT66WebView2Host::IsReady() const
{
	return Impl && Impl->View != nullptr;
}

#endif // PLATFORM_WINDOWS && T66_WITH_WEBVIEW2

