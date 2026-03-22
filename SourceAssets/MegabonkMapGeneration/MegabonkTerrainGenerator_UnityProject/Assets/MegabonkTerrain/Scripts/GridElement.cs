using System.Collections.Generic;
using UnityEngine;

public class GridElement : MonoBehaviour
{
    private Vector3Int coordinates;

    [SerializeField] private GameObject cube;
    [SerializeField] private GameObject slope;
    [SerializeField] private GameObject bottomPart;
    [SerializeField] private GameObject bottomPartBlockPrefab; 

    private static readonly Dictionary<Vector2Int, float> directionToYaw = new()
    {
        { Vector2Int.right,  90f },
        { Vector2Int.left,  270f },
        { Vector2Int.up,     0f },
        { Vector2Int.down, 180f }
    };

    public Vector3Int Coordinates { get => coordinates; set => coordinates = value; }
    public bool IsSlope => slope.activeSelf;

    public void SetElevation(int elevation, float yPosition)
    {
        coordinates.y = elevation;
        transform.position = new Vector3(transform.position.x, yPosition, transform.position.z);        
    }

    public void MakeSlope(Vector2Int direction)
    {
        cube.gameObject.SetActive(false);
        slope.gameObject.SetActive(true);
        slope.transform.localEulerAngles = new Vector3(0, directionToYaw[direction], 0);
    }    

    public void ConfigureBottomPart(float blockSpacingVertical)
    {        
        for (int i = 0; i < coordinates.y; i++)
        {            
            Instantiate(bottomPartBlockPrefab, transform.position + Vector3.down * (i + 1) * blockSpacingVertical, Quaternion.identity, bottomPart.transform);
        }
    }   
}