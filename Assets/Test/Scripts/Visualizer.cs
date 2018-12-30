using UnityEngine;
using UnityEngine.Audio;
using UnityEngine.Experimental.VFX;
#if UNITY_EDITOR
using UnityEditor;
#endif


[ExecuteInEditMode]
public class Visualizer : MonoBehaviour
{
    public VisualEffect visualEffect;
    public DecibelCalculator[] sourceSound;
    public float spawn = 4000.0f;
    public float radius = 0.5f;


    void OnEnable()
    {
#if UNITY_EDITOR
        if (!EditorApplication.isPlaying)
            EditorApplication.update += Update;
#endif
    }

    void OnDisable()
    {
#if UNITY_EDITOR
        if (!EditorApplication.isPlaying)
            EditorApplication.update -= Update;
#endif
    }

    void Update()
    {
        if (sourceSound != null && sourceSound.Length > 0 && visualEffect != null)
        {
            float db = sourceSound[0].dB;
            for (int i = 1; i < sourceSound.Length; ++i)
                db = Mathf.Max(db, sourceSound[i].dB);

            float n = Mathf.Pow(Mathf.Clamp((db + 250.0f) / 250.0f, 0.0f, 1.0f), 2.0f);
            visualEffect.SetFloat("Spawn", n * spawn);
            visualEffect.SetFloat("Radius", n * radius);
            visualEffect.SetFloat("Octave", 3.0f + n * 4.0f);
        }
    }
}
