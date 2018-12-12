using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using IST.RemoteTalk;


[ExecuteInEditMode]
[RequireComponent(typeof(AudioSource))]
public class TestAudioFilter : MonoBehaviour
{
    AudioSource m_audioSource;
    public bool m_playOnAudioRead;
    public bool m_playOnAudioFilterRead;
    public int m_cycle;
    public bool m_logging;


    void OnEnable()
    {
        m_audioSource = GetComponent<AudioSource>();
    }

    void OnDisable()
    {
    }

    void Update()
    {
        if (!m_audioSource.isPlaying && (m_playOnAudioRead || m_playOnAudioFilterRead))
        {
            //var clip = AudioClip.Create("TestAudioFilterAudio",
            //    44000,
            //    1, 44000, true,
            //    OnAudioRead, OnAudioSetPosition);
            //m_audioSource.clip = clip;
            m_audioSource.loop = true;
            m_audioSource.Play();
        }

        if (m_audioSource.isPlaying && (!m_playOnAudioRead && !m_playOnAudioFilterRead))
        {
            m_audioSource.Stop();
        }
    }


    void OnAudioRead(float[] data)
    {
        if (!m_playOnAudioRead)
        {
            for (int i = 0; i < data.Length; ++i)
                data[i] = 0.0f;
            return;
        }

        for (int i = 0; i < data.Length; ++i)
            data[i] = Mathf.Sin((float)(m_cycle + i) * 0.5f * Mathf.Deg2Rad) * 0.8f;
        m_cycle += data.Length;
    }

    void OnAudioSetPosition(int pos)
    {
    }

    void OnAudioFilterRead(float[] data, int channels)
    {
        if (!m_playOnAudioFilterRead)
            return;

        //if (m_logging)
        //    Debug.Log("OnAudioFilterRead() len: " + data.Length + ", channels: " + channels);

        for (int i = 0; i < data.Length; ++i)
            data[i] += Mathf.Sin((float)(m_cycle + i) * 0.5f * Mathf.Deg2Rad) * 0.2f;
        m_cycle += data.Length;
    }
}
