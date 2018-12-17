using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif
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
                if (m_delay < m_current.delay)
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

#if UNITY_EDITOR
    [MenuItem("Debug/Remote Talk/List All Casts", false, 10)]
    public static void ListAllCasts(MenuCommand menuCommand)
    {
        string result = "";
        foreach(var c in RemoteTalkProvider.allCasts) {
            result += c.name + "\n";
            result += "  host: " + c.host + "\n";
            result += "  params:\n";
            foreach (var pn in c.paramNames)
                result += "    " + pn + "\n";
            result += "\n";
        }
        Debug.Log(result);
    }
#endif

}
