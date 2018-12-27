using UnityEngine;
using UnityEngine.Audio;
#if UNITY_EDITOR
using UnityEditor;
#endif


[ExecuteInEditMode]
public class Visualizer : MonoBehaviour
{
    public DecibelCalculator sourceSound;
    public Material dstMaterial;
    public Transform dstTransform;

    Material m_material;
    public Color m_emission;

    void OnEnable()
    {
        m_material = new Material(dstMaterial);
        var renderer = GetComponent<MeshRenderer>();
        renderer.sharedMaterial = m_material;

        m_emission = m_material.GetColor("_EmissionColor");

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
        float v = Mathf.Clamp((sourceSound.dB / 150.0f + 0.5f) * 2.0f, 0.0f, 3.0f);
        m_material.SetColor("_EmissionColor", m_emission * v);
        dstTransform.localScale = new Vector3(v, v, v);
    }
}
