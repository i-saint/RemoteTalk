using UnityEngine;
using UnityEngine.Audio;


[ExecuteInEditMode]
public class EchoController : MonoBehaviour
{
    public AudioEchoFilter filter;

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
