using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using IST.RemoteTalk;


[ExecuteInEditMode]
[RequireComponent(typeof(AudioSource))]
public class DecibelCalculator : MonoBehaviour
{
    const float ZeroOffset = 1.5849e-13f;
    const float RefLevel = 0.70710678118f; // 1/sqrt(2)

    public float dB;
    float m_squareSum;
    int m_sampleCount;


    public bool bandPassFilter;
    [Range(0.0f, 1.0f)] public float cutoff = 0.5f;
    [Range(1.0f, 10.0f)] public float q = 1.0f;
    // DSP variables
    float m_vF;
    float m_vD;
    float m_vZ1;
    float m_vZ2;
    float m_vZ3;

    public float CutoffFrequency
    {
        get { return Mathf.Pow(2, 10 * cutoff - 10) * 15000; }
    }

    void Update()
    {
        if (m_sampleCount > 0)
        {
            var rms = Mathf.Min(1.0f, Mathf.Sqrt(m_squareSum / m_sampleCount));
            dB = 20.0f * Mathf.Log10(rms / RefLevel + ZeroOffset);

            m_squareSum = 0;
            m_sampleCount = 0;
        }

        if (bandPassFilter)
        {
            var f = 2 / 1.85f * Mathf.Sin(Mathf.PI * CutoffFrequency / AudioSettings.outputSampleRate);
            m_vD = 1 / q;
            m_vF = (1.85f - 0.75f * m_vD * f) * f;
        }
    }

    void OnAudioFilterRead(float[] data, int channels)
    {
        for (var i = 0; i < data.Length; i += channels)
        {
            var s = data[i];
            if (bandPassFilter)
            {
                var vZ1 = 0.5f * s;
                var vZ3 = m_vZ2 * m_vF + m_vZ3;
                var vZ2 = (vZ1 + m_vZ1 - vZ3 - m_vZ2 * m_vD) * m_vF + m_vZ2;

                s = vZ2;

                m_vZ1 = vZ1;
                m_vZ2 = vZ2;
                m_vZ3 = vZ3;
            }
            m_squareSum += s * s;
        }
        m_sampleCount += data.Length / channels;
    }
}
