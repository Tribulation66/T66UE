using UnityEngine;

public class ObjectSpawner : MonoBehaviour
{
    [SerializeField] private GameObject[] treePrefabs;
    [SerializeField] private GameObject[] stonePrefabs;
    [SerializeField] private GameObject[] logPrefabs;

    public void SpawnRandomObject(GridElement gridElement)
    {
        float objectToSpawn = Random.Range(0, 1f);
            if (objectToSpawn > 0.75f)
            {
                SpawnTree(gridElement);
            }
            else if (objectToSpawn > 0.5f)
            {
                SpawnStone(gridElement);
            }
            else if (objectToSpawn > 0.4f)
            {
                SpawnLog(gridElement);
            }
    }

    private GameObject SpawnRandomElementFromArray(GridElement gridElement, GameObject[] objectArray, float localScale, Vector3 localPosition, Vector3 localEulerAngles)
    {
        GameObject spawnedObject = Instantiate(objectArray[Random.Range(0, objectArray.Length)], gridElement.transform.position, Quaternion.identity, gridElement.transform);
        spawnedObject.transform.localScale = Vector3.one * localScale;
        spawnedObject.transform.localPosition = localPosition;
        spawnedObject.transform.localEulerAngles = localEulerAngles;
        return spawnedObject;
    }

    private void SpawnTree(GridElement gridElement)
    {        
        Vector3 position = new Vector3(Random.Range(-0.25f, 0.25f), 0.25f, Random.Range(-0.25f, 0.25f));
        Vector3 rotation = new Vector3(90f, Random.Range(0, 360f), 0);
        SpawnRandomElementFromArray(gridElement, treePrefabs, 0.25f, position, rotation);        
    }

    private void SpawnStone(GridElement gridElement)
    {        
        Vector3 position = new Vector3(Random.Range(-0.25f, 0.25f), 0.25f, Random.Range(-0.25f, 0.25f));
        Vector3 rotation = new Vector3(0, Random.Range(0, 360f), 0);
        SpawnRandomElementFromArray(gridElement, stonePrefabs, Random.Range(0.25f, 0.5f), position, rotation);                
    }

    private void SpawnLog(GridElement gridElement)
    {        
        Vector3 position = new Vector3(Random.Range(-0.25f, 0.25f), 0.32f, Random.Range(-0.25f, 0.25f));
        Vector3 rotation = new Vector3(0, Random.Range(0, 360f), 0);
        SpawnRandomElementFromArray(gridElement, logPrefabs, Random.Range(0.1f, 0.25f), position, rotation);        
    }
}
