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
        [SerializeField] List<Talk> m_talks;
        [SerializeField] bool m_isPlaying;
        [SerializeField] public int m_talkPos = 0;

        Talk m_current;
        RemoteTalkProvider m_prevProvider;

        public void Play()
        {
            m_isPlaying = true;
        }

        public void Stop()
        {
            if (m_isPlaying)
            {
                if (m_prevProvider != null)
                    m_prevProvider.Stop();
                m_isPlaying = false;
            }
        }


        void UpdateTalk()
        {
            if (!m_isPlaying)
                return;

            if ((m_prevProvider == null || m_prevProvider.isIdling) && m_talkPos < m_talks.Count)
            {
                m_current = m_talks[m_talkPos++];
                if (m_current != null)
                {
                    var provider = RemoteTalkProvider.FindByCast(m_current.castName);
                    if (provider != null)
                        provider.Talk(m_current);
                    m_prevProvider = provider;
                }
            }
        }

        void Update()
        {
            UpdateTalk();
        }
    }
}
