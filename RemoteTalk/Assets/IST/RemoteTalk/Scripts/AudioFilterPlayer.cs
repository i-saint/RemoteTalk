
using System;
using System.Collections.Generic;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace IST.RemoteTalk
{
    [ExecuteInEditMode]
    [RequireComponent(typeof(AudioSource))]
    public class AudioFilterPlayer : MonoBehaviour
    {

        [SerializeField] AudioSource m_baseAudioSource;
        AudioSource m_audioSource;
        rtAudioData m_data;
        bool m_playing = false;
        int m_sampleRate;
        double m_samplePos;
        bool m_finished;

        public AudioSource baseAudioSource
        {
            get { return m_baseAudioSource; }
            set { m_baseAudioSource = value; }
        }
        public bool isPlaying { get { return m_audioSource != null && m_audioSource.isPlaying; } }

        public void Play(rtAudioData data)
        {
            m_data = data;
            if (!m_data || m_audioSource == null)
                return;

            {
                var clip = AudioClip.Create("One", 44000, 1, 44000, false);
                var samples = new float[44000];
                for (int i = 0; i < 44000; ++i)
                    samples[i] = 1.0f;
                clip.SetData(samples, 0);
                m_audioSource.clip = clip;
            }

            m_audioSource.playOnAwake = false;
            m_audioSource.loop = true;
            if (m_baseAudioSource != null)
            {
                m_audioSource.outputAudioMixerGroup = m_baseAudioSource.outputAudioMixerGroup;
                m_audioSource.mute = m_baseAudioSource.mute;
                m_audioSource.bypassEffects = m_baseAudioSource.bypassEffects;
                m_audioSource.bypassListenerEffects = m_baseAudioSource.bypassListenerEffects;
                m_audioSource.bypassReverbZones = m_baseAudioSource.bypassReverbZones;
                m_audioSource.priority = m_baseAudioSource.priority;
                m_audioSource.volume = m_baseAudioSource.volume;
                m_audioSource.pitch = m_baseAudioSource.pitch;
                m_audioSource.panStereo = m_baseAudioSource.panStereo;
                m_audioSource.spatialBlend = m_baseAudioSource.spatialBlend;
                m_audioSource.reverbZoneMix = m_baseAudioSource.reverbZoneMix;
                m_audioSource.dopplerLevel = m_baseAudioSource.dopplerLevel;
                m_audioSource.spread = m_baseAudioSource.spread;
                m_audioSource.minDistance = m_baseAudioSource.minDistance;
                m_audioSource.maxDistance = m_baseAudioSource.maxDistance;
                m_audioSource.rolloffMode = m_baseAudioSource.rolloffMode;
            }
            m_audioSource.Play();

            m_sampleRate = AudioSettings.outputSampleRate;
            m_samplePos = 0.0;
            m_finished = false;
            m_playing = true;
        }

        private void OnEnable()
        {
            m_audioSource = GetComponent<AudioSource>();
        }

        void Update()
        {
            if (m_finished)
            {
                m_audioSource.Stop();
                m_playing = false;
                m_finished = false;
            }
        }

        void OnAudioFilterRead(float[] data, int channels)
        {
            if (!m_playing)
                return;

            var prev = m_samplePos;
            m_samplePos = m_data.Resample(data, m_sampleRate, channels, data.Length, m_samplePos);
            if (m_samplePos == prev)
                m_finished = true;
        }
    }
}
