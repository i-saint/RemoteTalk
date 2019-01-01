using UnityEngine;


[ExecuteInEditMode]
public class ReverbController : MonoBehaviour
{
    public AudioReverbFilter filter;

    void OnEnable()
    {
        if (filter != null)
            filter.enabled = true;
    }

    void OnDisable()
    {
        if (filter != null)
            filter.enabled = false;
    }
}
