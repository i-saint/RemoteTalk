using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace IST.RemoteTalk
{
    [ExecuteInEditMode]
    [AddComponentMenu("RemoteTalk/Script")]
    public class RemoteTalkScript : MonoBehaviour
    {
        [SerializeField] List<Talk> m_talks = new List<Talk>();
        [SerializeField] bool m_playOnStart;
        [SerializeField] int m_startPos;
        bool m_isPlaying;
        int m_talkPos = 0;
        Talk m_current;
        RemoteTalkProvider m_prevProvider;


        public bool playOnStart
        {
            get { return m_playOnStart; }
            set { m_playOnStart = value; }
        }
        public int startPosition
        {
            get { return m_startPos; }
            set { m_startPos = value; }
        }
        public int playPosition
        {
            get { return Mathf.Clamp(m_talkPos - 1, 0, m_talks.Count); }
            set { m_talkPos = value; }
        }
        public bool isPlaying
        {
            get { return m_isPlaying; }
        }


        public void Play()
        {
            m_isPlaying = true;
            m_talkPos = m_startPos;
        }

        public void Stop()
        {
            if (m_isPlaying)
            {
                if (m_prevProvider != null)
                    m_prevProvider.Stop();
                m_isPlaying = false;
                m_current = null;
                m_prevProvider = null;
            }
        }


        void UpdateTalk()
        {
            if (!m_isPlaying || !isActiveAndEnabled)
                return;

            if (m_prevProvider == null || m_prevProvider.isIdling)
            {
                if(m_talkPos >= m_talks.Count)
                {
                    m_isPlaying = false;
                }
                else
                {
                    m_current = m_talks[m_talkPos++];
                    if (m_current != null)
                    {
                        var provider = m_current.provider;
                        if (provider != null)
                            provider.Play(m_current);
                        m_prevProvider = provider;
                    }
                }
            }
        }


        void Start()
        {
            if (m_playOnStart)
                Start();
        }

        void Update()
        {
            UpdateTalk();
        }

        void OnEnable()
        {
#if UNITY_EDITOR
            if (!EditorApplication.isPlaying)
                EditorApplication.update += UpdateTalk;
#endif
        }

        void OnDisable()
        {
#if UNITY_EDITOR
            if (!EditorApplication.isPlaying)
                EditorApplication.update -= UpdateTalk;
#endif
        }

    }
}
