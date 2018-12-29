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

            float v = (Mathf.Clamp(db / 150.0f, -1.0f, 1.0f) * 0.5f + 0.5f) * spawn;
            visualEffect.SetFloat("Spawn", v);
        }
    }
}
