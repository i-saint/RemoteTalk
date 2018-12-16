using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using IST.RemoteTalk;

public class RemoteTalkDemo : MonoBehaviour
{
    [Serializable]
    public class Script
    {
        public rtTalkParams talkParams = rtTalkParams.defaultValue;
        public string text;
        public int delay = 0;
        public bool reverb;
        public bool echo;
    }

    public RemoteTalkClient m_remoteTalk;
    public AudioReverbFilter m_reverb;
    public AudioEchoFilter m_echo;
    public List<Script> m_scripts = new List<Script>();
    public int m_scriptPos = 0;
    public int m_delay;
    Script m_current;

    void Update()
    {
        if (m_remoteTalk.isIdling && m_scriptPos < m_scripts.Count)
        {
            if (m_current != null)
            {
                if(m_delay < m_current.delay)
                {
                    ++m_delay;
                    return;
                }
            }

            m_delay = 0;
            m_current = m_scripts[m_scriptPos++];
            if (m_reverb != null)
                m_reverb.enabled = m_current.reverb;
            if (m_echo != null)
                m_echo.enabled = m_current.echo;

            m_remoteTalk.talkParams = m_current.talkParams;
            m_remoteTalk.talkText = m_current.text;
            m_remoteTalk.Talk();
        }
    }
}
