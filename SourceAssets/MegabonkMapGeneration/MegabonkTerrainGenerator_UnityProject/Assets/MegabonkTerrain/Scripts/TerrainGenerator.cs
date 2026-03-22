using System.Collections.Generic;
using UnityEngine;

public class TerrainGenerator : MonoBehaviour
{
    [SerializeField] private GridElement blockPrefab;
    [SerializeField] private GameObject wallPrefab;
    [SerializeField] private ObjectSpawner objectSpawner;
    [SerializeField] private GameObject playerPrefab;
    
    [SerializeField] private int mapSize;
    [Range(0, 1f)] [SerializeField] private float hilliness = 0.5f;

    private const float blockSpacingHorizontal = 20f;
    private const float blockSpacingVertical = 12f;

    private GridElement[,] gridElements;
    private GridElement currentElement;
    private Vector2Int[] directions = {Vector2Int.right, Vector2Int.left, Vector2Int.up, Vector2Int.down};

    private Vector2Int lockedDirection = new Vector2Int();    

    void Start()
    {
        GenerateTerrain(Random.Range(0, mapSize), Random.Range(0, mapSize));
        Cursor.visible = false;
        Cursor.lockState = CursorLockMode.Locked;
        Instantiate(playerPrefab, GetGridElement(mapSize / 2, mapSize / 2).transform.position + Vector3.up * 10f, Quaternion.identity);
    }    

    private void GenerateTerrain(int startX, int startZ)
    {
        gridElements = new GridElement[mapSize, mapSize];
        CreateElement(startX, 0, startZ);
        currentElement = gridElements[startX, startZ];
        while (currentElement != null)
        {
            ExpandElement();
        }
        CreateWalls();        
    }
    
    public GridElement GetGridElement(int x, int z)
    {
        if (x < 0 || x >= mapSize || z < 0 || z >= mapSize) return null;        
        return gridElements[x, z];
    }

    private void ExpandElement()
    {
        Vector2Int direction = lockedDirection != Vector2Int.zero ? lockedDirection : GetRandomDirectionFromElement(currentElement);
        if (direction == Vector2Int.zero)
        {
            currentElement = FirstNonSlopeElementWithAvailableSpace();
            if (currentElement == null) return;
            direction = lockedDirection != Vector2Int.zero ? lockedDirection : GetRandomDirectionFromElement(currentElement);
        }

        currentElement = CreateElement(currentElement.Coordinates.x + direction.x, currentElement.Coordinates.y, currentElement.Coordinates.z + direction.y);
        
        if (CanRaiseElevationInDirection(currentElement, direction) && Random.value < (hilliness / 5f))
        {
            currentElement.SetElevation(currentElement.Coordinates.y + 1, (currentElement.Coordinates.y + 1) * blockSpacingVertical);
            currentElement.MakeSlope(direction);
            lockedDirection = direction;
        }
        else
        {
            lockedDirection = Vector2Int.zero;
            objectSpawner.SpawnRandomObject(currentElement);
        }
        currentElement.ConfigureBottomPart(blockSpacingVertical);
    }

    private GridElement FirstNonSlopeElementWithAvailableSpace()
    {
        for (int i = 0; i < mapSize; i++)
        {
            for (int j = 0; j < mapSize; j++)
            {
                if (gridElements[i, j] != null && !gridElements[i, j].IsSlope)
                {
                    foreach (Vector2Int dir in directions)
                    {
                        int newX = i + dir.x;
                        int newY = j + dir.y;
                        if (newX >= 0 && newX < mapSize && newY >= 0 && newY < mapSize && gridElements[newX, newY] == null)
                        {
                            return gridElements[i, j];
                        }
                        
                    }
                }
            }
        }
        return null;
    }
    
    private bool CanRaiseElevationInDirection(GridElement currentElement, Vector2Int direction)
    {
        Vector2Int target = new Vector2Int(currentElement.Coordinates.x + direction.x, currentElement.Coordinates.z + direction.y);
        if (target.x >= mapSize || target.x < 0 || target.y >= mapSize || target.y < 0) return false;        
        return gridElements[target.x, target.y] == null;
    }

    private GridElement CreateElement(int x, int y, int z)
    {
        GridElement element = Instantiate(blockPrefab, new Vector3(x * blockSpacingHorizontal, y * blockSpacingVertical, z * blockSpacingHorizontal), Quaternion.identity, transform);
        element.Coordinates = new Vector3Int(x, y, z);
        gridElements[x, z] = element;
        return element;
    }

    private Vector2Int GetRandomDirectionFromElement(GridElement element)
    {
        List<Vector2Int> directionPool = new List<Vector2Int>();
        
        foreach (Vector2Int dir in directions)
        {
            int newX = element.Coordinates.x + dir.x;
            int newY = element.Coordinates.z + dir.y;
            if (newX >= 0 && newY >= 0 && newX < mapSize && newY < mapSize && gridElements[newX, newY] == null)
            {
                directionPool.Add(dir);
            }
        }

        if (directionPool.Count > 0)
        {
            return directionPool[Random.Range(0, directionPool.Count)];
        }
        else
        {            
            return new Vector2Int();
        }
    }

    private void CreateWalls()
    {
        float wallOffset = mapSize / 2 * blockSpacingHorizontal - (mapSize % 2 == 0 ? blockSpacingHorizontal / 2f : 0);        
        Vector3 wallPositionNorthSouth = new Vector3(wallOffset, 0, 0);
        Vector3 wallScaleNorthSouth = new Vector3(mapSize * blockSpacingHorizontal, 50 * blockSpacingVertical, blockSpacingHorizontal);
        Vector3 wallPositionEastWest = new Vector3(0, 0, wallOffset);
        Vector3 wallScaleEastWest = new Vector3(blockSpacingHorizontal, 50 * blockSpacingVertical, mapSize * blockSpacingHorizontal);

        GameObject wallNorth = Instantiate(wallPrefab, transform.position, Quaternion.identity, transform);
        wallNorth.transform.position = wallPositionNorthSouth + Vector3.forward * mapSize * blockSpacingHorizontal;
        wallNorth.transform.localScale = wallScaleNorthSouth;
        
        GameObject wallSouth = Instantiate(wallPrefab, transform.position, Quaternion.identity, transform);
        wallSouth.transform.position = wallPositionNorthSouth + Vector3.forward * -blockSpacingHorizontal;
        wallSouth.transform.localScale = wallScaleNorthSouth;        
        
        GameObject wallEast = Instantiate(wallPrefab, transform.position, Quaternion.identity, transform);
        wallEast.transform.position = wallPositionEastWest + Vector3.right * mapSize * blockSpacingHorizontal;
        wallEast.transform.localScale = wallScaleEastWest;
        
        GameObject wallWest = Instantiate(wallPrefab, transform.position, Quaternion.identity, transform);
        wallWest.transform.position = wallPositionEastWest + Vector3.right * -blockSpacingHorizontal;
        wallWest.transform.localScale = wallScaleEastWest;
    }

    
}
