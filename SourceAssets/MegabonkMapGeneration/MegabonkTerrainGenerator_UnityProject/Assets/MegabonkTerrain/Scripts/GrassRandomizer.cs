using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GrassRandomizer : MonoBehaviour
{
    [SerializeField] private Transform[] grassObjects;
    [SerializeField] private Material[] grassMaterials;
    [SerializeField] private float grassMinScale;
    [SerializeField] private float grassMaxScale;
    [SerializeField] private float xPositionVariation;
    [SerializeField] private float zPositionVariation;

    void Awake()
    {
        RandomizeGrass();
    }

    private void RandomizeGrass()
    {
        foreach (Transform grass in grassObjects)
        {
            MeshRenderer grassRenderer = grass.GetComponentInChildren<MeshRenderer>();
            grassRenderer.material = grassMaterials[Random.Range(0, grassMaterials.Length)];
            grass.localScale = new Vector3(1f, Random.Range(grassMinScale, grassMaxScale), 1f);
            grass.localPosition = new Vector3(
                Random.Range(-xPositionVariation, xPositionVariation),
                grass.localPosition.y,
                Random.Range(-zPositionVariation, zPositionVariation)
            );
            grass.localEulerAngles = new Vector3(grass.localEulerAngles.x, Random.Range(0, 360f), grass.localEulerAngles.z);
        }        
    }
}
