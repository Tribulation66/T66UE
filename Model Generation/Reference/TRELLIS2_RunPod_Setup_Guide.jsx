import { useState, useCallback } from "react";

const CodeBlock = ({ code, label }) => {
  const [copied, setCopied] = useState(false);
  const handleCopy = useCallback(() => {
    navigator.clipboard.writeText(code).then(() => {
      setCopied(true);
      setTimeout(() => setCopied(false), 2000);
    });
  }, [code]);

  return (
    <div style={{ position: "relative", background: "#0d1117", border: "1px solid #30363d", borderRadius: 8, marginBottom: 16, overflow: "hidden" }}>
      <div style={{ display: "flex", justifyContent: "space-between", alignItems: "center", padding: "6px 12px", background: "#161b22", borderBottom: "1px solid #30363d" }}>
        <span style={{ fontSize: 11, color: "#8b949e", textTransform: "uppercase", letterSpacing: "0.05em", fontWeight: 600 }}>{label || "bash"}</span>
        <button onClick={handleCopy} style={{ background: copied ? "#238636" : "#30363d", color: "#e6edf3", border: "none", borderRadius: 4, padding: "4px 10px", fontSize: 12, cursor: "pointer", transition: "all 0.2s", fontFamily: "inherit" }}>
          {copied ? "✓ Copied" : "Copy"}
        </button>
      </div>
      <pre style={{ margin: 0, padding: "14px 16px", overflowX: "auto" }}>
        <code style={{ fontFamily: "'SF Mono', 'Fira Code', 'Cascadia Code', monospace", fontSize: 13, lineHeight: 1.6, color: "#e6edf3" }}>{code}</code>
      </pre>
    </div>
  );
};

const Callout = ({ type, children }) => {
  const styles = {
    warning: { bg: "#d2992215", border: "#d2992244", left: "#d29922", icon: "⚠" },
    success: { bg: "#3fb95015", border: "#3fb95044", left: "#3fb950", icon: "✓" },
    note: { bg: "#1f6feb22", border: "#58a6ff33", left: "#58a6ff", icon: "ℹ" },
  };
  const s = styles[type] || styles.note;
  return (
    <div style={{ background: s.bg, border: `1px solid ${s.border}`, borderLeft: `3px solid ${s.left}`, borderRadius: 6, padding: "12px 16px", marginBottom: 16, fontSize: 14, lineHeight: 1.6 }}>
      <span style={{ fontWeight: 700, marginRight: 6 }}>{s.icon}</span>
      {children}
    </div>
  );
};

const Tag = ({ children }) => (
  <span style={{ display: "inline-block", fontSize: 11, padding: "2px 7px", borderRadius: 3, fontWeight: 600, textTransform: "uppercase", letterSpacing: "0.03em", background: "#d2992225", color: "#d29922", border: "1px solid #d2992244", marginLeft: 8 }}>{children}</span>
);

const IC = ({ children }) => (
  <code style={{ background: "#1c2330", padding: "2px 6px", borderRadius: 4, fontFamily: "'SF Mono', 'Fira Code', monospace", fontSize: "0.88em", color: "#58a6ff" }}>{children}</code>
);

const Table = ({ headers, rows }) => (
  <div style={{ overflowX: "auto", marginBottom: 16 }}>
    <table style={{ width: "100%", borderCollapse: "collapse", fontSize: 14 }}>
      <thead><tr>{headers.map((h, i) => <th key={i} style={{ background: "#161b22", textAlign: "left", padding: "10px 12px", border: "1px solid #30363d", fontWeight: 600, color: "#8b949e", fontSize: 12, textTransform: "uppercase", letterSpacing: "0.04em" }}>{h}</th>)}</tr></thead>
      <tbody>{rows.map((row, ri) => <tr key={ri} style={{ background: ri % 2 === 0 ? "transparent" : "#161b22" }}>{row.map((cell, ci) => <td key={ci} style={{ padding: "8px 12px", border: "1px solid #30363d", lineHeight: 1.5 }}>{cell}</td>)}</tr>)}</tbody>
    </table>
  </div>
);

const Step = ({ num, title, children }) => (
  <div style={{ marginTop: 40 }}>
    <h2 style={{ fontSize: 22, fontWeight: 600, marginBottom: 12, display: "flex", alignItems: "center", gap: 10 }}>
      <span style={{ background: "#58a6ff", color: "#0d1117", fontSize: 13, fontWeight: 700, padding: "3px 9px", borderRadius: 4, flexShrink: 0 }}>{num}</span>
      {title}
    </h2>
    {children}
  </div>
);

const HR = () => <hr style={{ border: "none", borderTop: "1px solid #30363d", margin: "40px 0" }} />;
const H3 = ({ children }) => <h3 style={{ fontSize: 15, fontWeight: 600, marginTop: 16, marginBottom: 8, color: "#8b949e" }}>{children}</h3>;
const P = ({ children }) => <p style={{ marginBottom: 12 }}>{children}</p>;

const SERVER_SCRIPT = `"""
TRELLIS.2 API Server for Chadpocalypse Pipeline
Quality settings: 20 steps per stage, texture guidance 2.0
"""
import os
os.environ['OPENCV_IO_ENABLE_OPENEXR'] = '1'
os.environ["PYTORCH_CUDA_ALLOC_CONF"] = "expandable_segments:True"
import io
import time
import cv2
import torch
import imageio
from PIL import Image
from flask import Flask, request, send_file, jsonify
from trellis2.pipelines import Trellis2ImageTo3DPipeline
from trellis2.utils import render_utils
from trellis2.renderers import EnvMap
import o_voxel

app = Flask(__name__)

print("[TRELLIS] Loading pipeline...")
pipeline = Trellis2ImageTo3DPipeline.from_pretrained("microsoft/TRELLIS.2-4B")
pipeline.cuda()
print("[TRELLIS] Pipeline loaded!")

envmap = EnvMap(torch.tensor(
    cv2.cvtColor(
        cv2.imread('assets/hdri/forest.exr', cv2.IMREAD_UNCHANGED),
        cv2.COLOR_BGR2RGB
    ),
    dtype=torch.float32, device='cuda'
))
print("[TRELLIS] Environment map loaded!")

@app.route('/health', methods=['GET'])
def health():
    return jsonify({
        "status": "ok",
        "gpu": torch.cuda.get_device_name(0),
        "vram_total_gb": round(torch.cuda.get_device_properties(0).total_memory / 1e9, 1),
        "vram_used_gb": round(torch.cuda.memory_allocated() / 1e9, 1),
    })

@app.route('/generate', methods=['POST'])
def generate():
    start_time = time.time()

    image_data = request.get_data()
    if not image_data:
        return jsonify({"error": "No image data received"}), 400

    image = Image.open(io.BytesIO(image_data)).convert("RGBA")

    seed = int(request.headers.get('X-Seed', str(torch.randint(0, 2**31, (1,)).item())))
    texture_size = int(request.headers.get('X-Texture-Size', '2048'))
    decimation = int(request.headers.get('X-Decimation', '200000'))

    print(f"[TRELLIS] Generating: seed={seed}, tex={texture_size}, decim={decimation}")

    outputs, latents = pipeline.run(
        image,
        seed=seed,
        preprocess_image=True,
        return_latent=True,
        sparse_structure_sampler_params={
            "steps": 20,
        },
        shape_slat_sampler_params={
            "steps": 20,
        },
        tex_slat_sampler_params={
            "steps": 20,
            "guidance_strength": 2.0,
        },
    )
    mesh = outputs[0]
    res = latents[2]
    mesh.simplify(16777216)

    job_id = f"{int(time.time())}_{seed}"
    tmp_path = f"/tmp/{job_id}.glb"

    glb = o_voxel.postprocess.to_glb(
        vertices=mesh.vertices,
        faces=mesh.faces,
        attr_volume=mesh.attrs,
        coords=mesh.coords,
        attr_layout=pipeline.pbr_attr_layout,
        grid_size=res,
        aabb=[[-0.5, -0.5, -0.5], [0.5, 0.5, 0.5]],
        decimation_target=decimation,
        texture_size=texture_size,
        remesh=True,
        remesh_band=1,
        remesh_project=0,
        verbose=True,
    )
    glb.export(tmp_path, extension_webp=True)
    torch.cuda.empty_cache()

    elapsed = time.time() - start_time
    print(f"[TRELLIS] Done in {elapsed:.1f}s")

    response = send_file(tmp_path, mimetype='model/gltf-binary',
                         as_attachment=True,
                         download_name=f"trellis_{job_id}.glb")

    @response.call_on_close
    def cleanup():
        try:
            os.remove(tmp_path)
        except:
            pass

    return response

if __name__ == '__main__':
    print("[TRELLIS] Starting server on port 8000...")
    app.run(host='0.0.0.0', port=8000, threaded=False)`;

export default function TrellisGuide() {
  return (
    <div style={{ fontFamily: "-apple-system, BlinkMacSystemFont, 'Segoe UI', Helvetica, Arial, sans-serif", background: "#0d1117", color: "#e6edf3", lineHeight: 1.7, padding: "32px 24px", maxWidth: 880, margin: "0 auto", minHeight: "100vh" }}>
      <h1 style={{ fontSize: 28, fontWeight: 700, marginBottom: 4, borderBottom: "1px solid #30363d", paddingBottom: 12 }}>TRELLIS.2 RunPod Setup Guide</h1>
      <p style={{ color: "#8b949e", fontSize: 15, marginBottom: 24 }}>Chadpocalypse 3D Generation Pipeline</p>

      <div style={{ background: "#161b22", border: "1px solid #30363d", borderRadius: 8, padding: "16px 20px", marginBottom: 32 }}>
        <P><strong style={{ color: "#58a6ff" }}>What this does:</strong> Runs TRELLIS.2 (4B parameter image-to-3D model) on a RunPod GPU pod, accessible via PowerShell from your PC. Send a PNG image, get back a .GLB 3D model with full PBR textures.</P>
        <p><strong style={{ color: "#58a6ff" }}>Performance:</strong> ~50 seconds per model at 200k faces / 2048 texture on RTX A6000.</p>
      </div>

      <Step num="1" title="Create RunPod Pod">
        <ul style={{ paddingLeft: 24, marginBottom: 12 }}>
          <li><strong>GPU:</strong> RTX A6000 (48GB VRAM) — or any GPU with 24GB+ VRAM</li>
          <li><strong>Template:</strong> RunPod PyTorch 2.4.0, CUDA 12.4</li>
          <li><strong>Expose Ports:</strong> <IC>8000</IC> (HTTP) and <IC>8888</IC> (JupyterLab)</li>
          <li><strong>Volume:</strong> Not required (everything installs to <IC>/workspace</IC>)</li>
        </ul>
        <CodeBlock label="Template ID" code="runpod/pytorch:2.4.0-py3.11-cuda12.4.1-devel-ubuntu22.04" />
      </Step>

      <Step num="2" title="SSH In">
        <P>Get the SSH details from RunPod's "Connect" tab on your pod.</P>
        <CodeBlock code={`ssh root@<POD_IP> -p <SSH_PORT> -i ~/.ssh/id_ed25519`} />
      </Step>

      <Step num="3" title="Clone TRELLIS.2">
        <P>The <IC>--recursive</IC> flag pulls the Eigen submodule needed for o-voxel.</P>
        <CodeBlock code="cd /workspace && git clone --recursive https://github.com/microsoft/TRELLIS.2.git" />
      </Step>

      <Step num="4" title="Install Miniconda">
        <CodeBlock code={`wget -q https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh -O /tmp/miniconda.sh
bash /tmp/miniconda.sh -b -p /workspace/miniconda3
eval "$(/workspace/miniconda3/bin/conda shell.bash hook)"`} />
        <Callout type="note">If you get "File or directory already exists", run <IC>rm -rf /workspace/miniconda3</IC> first, then re-run the install.</Callout>
      </Step>

      <Step num="5" title="Run TRELLIS.2 Setup Script">
        <CodeBlock code={`cd /workspace/TRELLIS.2
. ./setup.sh --new-env --basic --flash-attn --nvdiffrast --nvdiffrec --cumesh --o-voxel --flexgemm`} />
        <P>This creates a <IC>trellis2</IC> conda environment and compiles all native extensions. <Tag>15-30 min</Tag></P>
        <Callout type="note">Your SSH might disconnect during compilation — that's fine, the process continues on the pod. Reconnect and check with <IC>ps aux | grep setup</IC>.</Callout>
        <P>After it finishes, activate the environment:</P>
        <CodeBlock code={`eval "$(/workspace/miniconda3/bin/conda shell.bash hook)"
conda activate trellis2`} />
      </Step>

      <Step num="6" title="Install Flash Attention (Manual)">
        <P>The setup script may fail to build flash-attn automatically. Install it manually:</P>
        <CodeBlock code={`pip install psutil
pip install flash-attn==2.7.3 --no-build-isolation`} />
        <P>This compilation takes ~10 minutes. <Tag>~10 min</Tag></P>
      </Step>

      <Step num="7" title="Verify All Modules">
        <CodeBlock label="python" code={`python -c "
import torch; print('torch OK')
import nvdiffrast; print('nvdiffrast OK')
import cumesh; print('cumesh OK')
import o_voxel; print('o_voxel OK')
import flash_attn; print('flash_attn OK')
from trellis2.pipelines import Trellis2ImageTo3DPipeline; print('Pipeline imports OK')
"`} />
        <Callout type="success">You should see all OK with the message: <IC>[SPARSE] Conv backend: flex_gemm; Attention backend: flash_attn</IC></Callout>
      </Step>

      <Step num="8" title="HuggingFace Authentication">
        <P>TRELLIS.2 needs access to two gated models. Accept access for both in your browser (logged into HuggingFace):</P>
        <ul style={{ paddingLeft: 24, marginBottom: 12 }}>
          <li><IC>facebook/dinov3-vitl16-pretrain-lvd1689m</IC></li>
          <li><IC>briaai/RMBG-2.0</IC></li>
        </ul>
        <P>Then log in on the pod:</P>
        <CodeBlock code={`python -c "from huggingface_hub import login; login(token='YOUR_HF_TOKEN_HERE')"`} />
      </Step>

      <Step num="9" title="Install Flask, rembg, and onnxruntime">
        <CodeBlock code={`pip install flask
pip install onnxruntime
pip install rembg`} />
        <P><IC>rembg</IC> is the background removal library. It replaces RMBG-2.0/BiRefNet which has a compatibility issue with transformers 5.x (meta tensor crash). <IC>onnxruntime</IC> is a required dependency for rembg's inference engine.</P>
      </Step>

      <Step num="10" title="Background Removal Override">
        <P>RMBG-2.0 crashes with <IC>Tensor.item() cannot be called on meta tensors</IC> due to transformers 5.x incompatibility. We replace it with rembg.</P>
        <CodeBlock code={`cat > /workspace/TRELLIS.2/trellis2/pipelines/rembg/BiRefNet.py << 'EOF'
from typing import *
from PIL import Image
from rembg import remove


class BiRefNet:
    def __init__(self, model_name: str = "ZhengPeng7/BiRefNet"):
        pass

    def to(self, device: str):
        return self

    def cuda(self):
        return self

    def cpu(self):
        return self

    def __call__(self, image: Image.Image) -> Image.Image:
        return remove(image)
EOF`} />
        <P>Verify the override:</P>
        <CodeBlock code="cat /workspace/TRELLIS.2/trellis2/pipelines/rembg/BiRefNet.py" />
        <Callout type="note"><strong>Why this works:</strong> TRELLIS's pipeline imports <IC>BiRefNet</IC> from this file. By replacing it with a class that calls <IC>rembg.remove()</IC>, we get proper background removal without touching any other TRELLIS code.</Callout>
        <Callout type="warning">Without background removal, TRELLIS interprets the background as part of the 3D object and generates a floor/platform beneath the character.</Callout>
      </Step>

      <Step num="11" title="Fix cfg_strength Bug">
        <P>There's a known issue where <IC>cfg_strength</IC> (from TRELLIS v1's shared sparse_structure_decoder model) leaks through kwargs and crashes the sampler.</P>
        <CodeBlock code={`sed -i 's/return model(x_t, t, cond, \\*\\*kwargs)/kwargs.pop("cfg_strength", None); return model(x_t, t, cond, **kwargs)/' /workspace/TRELLIS.2/trellis2/pipelines/samplers/flow_euler.py`} />
        <P>Verify:</P>
        <CodeBlock code={`grep "cfg_strength" /workspace/TRELLIS.2/trellis2/pipelines/samplers/flow_euler.py`} />
      </Step>

      <Step num="12" title="Create the Server Script">
        <P>Open <strong>JupyterLab</strong> (port 8888), navigate to <IC>/workspace/TRELLIS.2/</IC>, right-click → <strong>New File</strong>, name it <IC>trellis_server.py</IC>, open it and paste the following:</P>
        <CodeBlock label="python — trellis_server.py" code={SERVER_SCRIPT} />
        <P>Save the file (<IC>Ctrl+S</IC>).</P>

        <H3>Server Defaults</H3>
        <Table
          headers={["Setting", "Value", "Notes"]}
          rows={[
            [<IC>preprocess_image</IC>, <IC>True</IC>, "rembg removes background"],
            [<IC>X-Decimation</IC>, <IC>200000</IC>, "200k faces"],
            [<IC>X-Texture-Size</IC>, <IC>2048</IC>, "Plenty for pixelated style"],
            ["Output location", <IC>/tmp/</IC>, "Sent to client then cleaned up"],
          ]}
        />
      </Step>

      <Step num="13" title="Start the Server">
        <CodeBlock code={`cd /workspace/TRELLIS.2
eval "$(/workspace/miniconda3/bin/conda shell.bash hook)"
conda activate trellis2
python /workspace/TRELLIS.2/trellis_server.py`} />
        <P><strong>First run</strong> downloads ~15GB of model weights + the rembg U2Net model (~176MB). Subsequent starts are fast (~30 seconds).</P>
        <Callout type="success">
          You should see:<br />
          <IC>[TRELLIS] Pipeline loaded!</IC><br />
          <IC>[TRELLIS] Environment map loaded!</IC><br />
          <IC>[TRELLIS] Starting server on port 8000...</IC>
        </Callout>
      </Step>

      <Step num="14" title="Generate from PowerShell">
        <P>RunPod's proxy has a ~100 second timeout which is too short for generation. Use SSH port forwarding to bypass it. <strong>You need 3 PowerShell windows:</strong></P>

        <H3>PowerShell 1 — Server (already running from Step 13)</H3>

        <H3>PowerShell 2 — SSH Tunnel (leave running)</H3>
        <CodeBlock label="powershell" code={`ssh -L 8000:localhost:8000 root@<POD_IP> -p <SSH_PORT> -i ~/.ssh/id_ed25519 -N`} />
        <P>Forwards your local port 8000 to the pod. The <IC>-N</IC> flag means no shell — it just sits there silently.</P>

        <H3>PowerShell 3 — Generate</H3>
        <P><strong>Using curl.exe (recommended for batch generation):</strong></P>
        <CodeBlock label="powershell" code={`curl.exe -X POST -H "Content-Type: image/png" -H "X-Seed: 42" -H "X-Decimation: 200000" -H "X-Texture-Size: 2048" --data-binary "@C:\\TRELLIS\\images\\Test.png" --output "C:\\TRELLIS\\outputs\\Test.glb" --max-time 600 http://localhost:8000/generate`} />

        <P><strong>Health check:</strong></P>
        <CodeBlock label="powershell" code={`Invoke-RestMethod -Uri "http://localhost:8000/health" -Method Get`} />
      </Step>

      <HR />

      <h2 style={{ fontSize: 22, fontWeight: 600, marginBottom: 12 }}>Quick Reference: Reconnecting</h2>
      <P>If your SSH disconnects:</P>
      <CodeBlock code={`ssh root@<POD_IP> -p <SSH_PORT> -i ~/.ssh/id_ed25519
eval "$(/workspace/miniconda3/bin/conda shell.bash hook)" && conda activate trellis2 && cd /workspace/TRELLIS.2`} />
      <Callout type="warning"><strong>DO NOT</strong> accidentally run <IC>pip install transformers==4.48.0</IC> — the environment needs <IC>transformers==5.2.0</IC> for DINOv3 support.</Callout>

      <HR />

      <h2 style={{ fontSize: 22, fontWeight: 600, marginBottom: 12 }}>Troubleshooting</h2>
      <Table
        headers={["Problem", "Solution"]}
        rows={[
          [<IC>Port 8000 is in use</IC>, <span><IC>pkill -9 -f python; sleep 1</IC> then restart</span>],
          [<span><IC>DINOv3ViTModel</IC> import error</span>, <span>Wrong transformers version. Run <IC>pip install transformers==5.2.0</IC></span>],
          [<IC>Tensor.item() on meta tensors</IC>, "BiRefNet override not applied (Step 10)"],
          [<span><IC>cfg_strength</IC> unexpected keyword</span>, "Sampler fix not applied (Step 11)"],
          [<span><IC>No module named 'onnxruntime'</IC></span>, <span>Run <IC>pip install onnxruntime</IC> (Step 9)</span>],
          [<IC>The operation has timed out</IC>, "SSH tunnel not running. Restart tunnel (Step 14)"],
          [<IC>connection was closed unexpectedly</IC>, "RunPod proxy timeout. Use SSH tunnel (Step 14)"],
          ["xatlas very slow (>5 min)", "Reduce decimation. 200k = ~15s, 500k = ~27s, 1M = 5-15min"],
          ["Platform/floor under character", "Background removal not working. Check Step 10. Use white backgrounds"],
          ["Dark/oversaturated textures", "Non-default sampler params. Reset to baseline"],
          ["Miniconda install 'already exists'", <span><IC>rm -rf /workspace/miniconda3</IC> then re-run install</span>],
          ["SSH drops during compilation", <span>Process continues on pod. Check with <IC>ps aux | grep setup</IC></span>],
        ]}
      />

      <HR />

      <h2 style={{ fontSize: 22, fontWeight: 600, marginBottom: 12 }}>Parameter Guide</h2>

      <H3>Request Headers</H3>
      <Table
        headers={["Header", "Default", "Description"]}
        rows={[
          [<IC>X-Seed</IC>, "random", "Random seed for reproducible generation"],
          [<IC>X-Texture-Size</IC>, "2048", "Texture resolution: 1024, 2048, 4096"],
          [<IC>X-Decimation</IC>, "200000", "Target face count"],
          [<IC>X-Steps</IC>, "12", "Sampling steps. 12 = default, 24 = higher quality"],
        ]}
      />

      <H3>Pipeline Defaults (from pipeline.json)</H3>
      <Table
        headers={["Stage", "Steps", "Guidance", "Rescale", "Interval", "rescale_t"]}
        rows={[
          ["Sparse Structure", "12", "7.5", "0.7", "[0.6, 1.0]", "5.0"],
          ["Shape SLat", "12", "7.5", "0.5", "[0.6, 1.0]", "3.0"],
          ["Texture SLat", "12", "1.0", "0.0", "[0.6, 0.9]", "3.0"],
        ]}
      />

      <HR />

      <h2 style={{ fontSize: 22, fontWeight: 600, marginBottom: 12 }}>Input Image Guidelines</h2>
      <ul style={{ paddingLeft: 24, marginBottom: 16 }}>
        <li><strong>White background</strong> is recommended — rembg cleanly removes it</li>
        <li><strong>Transparent PNGs</strong> can cause issues if the subject has white/light areas</li>
        <li><strong>Black backgrounds</strong> should be avoided — rembg struggles with dark-on-dark</li>
        <li>Show the full character in a <strong>T-pose or neutral stance</strong> for best results</li>
        <li>For best results, generate <strong>head and body separately</strong> then combine in Blender</li>
      </ul>

      <HR />

      <h2 style={{ fontSize: 22, fontWeight: 600, marginBottom: 12 }}>Files Modified on Pod</h2>
      <Table
        headers={["File", "What Changed", "Why"]}
        rows={[
          [<IC>trellis2/pipelines/rembg/BiRefNet.py</IC>, "Replaced with rembg wrapper", "RMBG-2.0 crashes with transformers 5.x"],
          [<IC>trellis2/pipelines/samplers/flow_euler.py</IC>, <span>Added <IC>kwargs.pop("cfg_strength", None)</IC></span>, "v1 model leaks old parameter name"],
          [<IC>trellis_server.py</IC>, "New file — Flask API server (via JupyterLab)", "Custom server for pipeline integration"],
        ]}
      />

      <div style={{ height: 60 }} />
    </div>
  );
}
