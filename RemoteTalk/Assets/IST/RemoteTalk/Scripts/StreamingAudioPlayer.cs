
using System;
using System.Collections.Generic;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace IST.RemoteTalk
{
    [ExecuteInEditMode]
    public class StreamingAudioPlayer : MonoBehaviour
    {
        public class Player
        {
            public AudioSource audioSource;
            public double startTime;
            public double endTime;

            public double duration { get { return endTime - startTime; } }
            public bool isPlaying { get { return audioSource.isPlaying; } }
        }


        [SerializeField] AudioSource m_baseAudioSource;
        LinkedList<Player> m_scheduledPlayers = new LinkedList<Player>();
        LinkedList<Player> m_idlingPlayers = new LinkedList<Player>();

        public AudioSource baseAudioSource
        {
            get { return m_baseAudioSource; }
            set { m_baseAudioSource = value; }
        }
        public bool isPlaying { get { return m_scheduledPlayers.Count > 0; } }

        public void Play(AudioClip clip)
        {
            if (clip == null)
                return;

            Player player;
            if (m_idlingPlayers.Count > 0)
            {
                player = m_idlingPlayers.First.Value;
                m_idlingPlayers.RemoveFirst();
            }
            else
            {
                player = new Player();
                GameObject child = new GameObject("Player");
                child.hideFlags = HideFlags.DontSave;
                child.transform.parent = gameObject.transform;
                player.audioSource = child.AddComponent<AudioSource>();
            }

            // setup AudioSource
            player.audioSource.playOnAwake = false;
            player.audioSource.loop = false;
            if (m_baseAudioSource != null)
            {
                player.audioSource.outputAudioMixerGroup = m_baseAudioSource.outputAudioMixerGroup;
                player.audioSource.mute = m_baseAudioSource.mute;
                player.audioSource.bypassEffects = m_baseAudioSource.bypassEffects;
                player.audioSource.bypassListenerEffects = m_baseAudioSource.bypassListenerEffects;
                player.audioSource.bypassReverbZones = m_baseAudioSource.bypassReverbZones;
                player.audioSource.priority = m_baseAudioSource.priority;
                player.audioSource.volume = m_baseAudioSource.volume;
                player.audioSource.pitch = m_baseAudioSource.pitch;
                player.audioSource.panStereo = m_baseAudioSource.panStereo;
                player.audioSource.spatialBlend = m_baseAudioSource.spatialBlend;
                player.audioSource.reverbZoneMix = m_baseAudioSource.reverbZoneMix;
                player.audioSource.dopplerLevel = m_baseAudioSource.dopplerLevel;
                player.audioSource.spread = m_baseAudioSource.spread;
                player.audioSource.minDistance = m_baseAudioSource.minDistance;
                player.audioSource.maxDistance = m_baseAudioSource.maxDistance;
                player.audioSource.rolloffMode = m_baseAudioSource.rolloffMode;
            }

            player.audioSource.clip = clip;
            if (m_scheduledPlayers.Count == 0)
            {
                // first clip. play immediately
                player.startTime = AudioSettings.dspTime;
                player.endTime = player.startTime + clip.length;
                player.audioSource.Play();
            }
            else
            {
                // schedule clip
                player.startTime = m_scheduledPlayers.Last.Value.endTime;
                player.endTime = player.startTime + clip.length;
                player.audioSource.PlayScheduled(player.startTime);
            }
            m_scheduledPlayers.AddLast(player);
        }

        void UpdatePlayers()
        {
            if (m_scheduledPlayers.Count == 0)
                return;

            while (m_scheduledPlayers.Count > 0)
            {
                var p = m_scheduledPlayers.First.Value;
                if (!p.isPlaying)
                {
                    p.audioSource.clip = null;
                    m_idlingPlayers.AddLast(p);
                    m_scheduledPlayers.RemoveFirst();
                }
                else
                {
                    break;
                }
            }
        }

        void Update()
        {
            UpdatePlayers();
        }
    }
}
