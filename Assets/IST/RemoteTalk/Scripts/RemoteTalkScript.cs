using System;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.IO;
using System.Collections.Generic;
using UnityEngine;
#if UNITY_2017_1_OR_NEWER
using UnityEngine.Playables;
using UnityEngine.Timeline;
#endif
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
        int m_talkPos = 0;
        float m_wait = 0.0f;
#if UNITY_EDITOR
        double m_prevTime;
#endif
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
        public bool isPlaying { get; private set; }


        public void Play()
        {
            isPlaying = true;
            m_talkPos = m_startPos;
            m_wait = 0.0f;
        }

        public void Stop()
        {
            if (isPlaying)
            {
                if (m_prevProvider != null)
                    m_prevProvider.Stop();
                isPlaying = false;
                m_current = null;
                m_prevProvider = null;
            }
        }

        static List<TalkParam> ExtractTalkParams(ref string line, ref float wait, bool clearParamBlock = false)
        {
            var rxParamBlock = new Regex(@"\{([^}]+)\}\s*");
            var rxParam = new Regex(@"([^ ]+?)\s*:\s*([\d.]+)");

            var ret = new List<TalkParam>();
            var block = rxParamBlock.Match(line);
            if (block.Success)
            {
                var pairs = block.Groups[1].Value.Split(',');
                foreach (var pair in pairs)
                {
                    var matcheParam = rxParam.Match(pair);
                    if (matcheParam.Success)
                    {
                        var name = matcheParam.Groups[1].Value;
                        var value = float.Parse(matcheParam.Groups[2].Value);
                        if (name == "wait")
                            wait = Mathf.Max(0.0f, value);
                        else
                            ret.Add(new TalkParam
                            {
                                name = name,
                                value = value,
                            });
                    }
                }

                if (clearParamBlock)
                    line = rxParamBlock.Replace(line, "");
            }
            return ret;
        }

        public static List<Talk> TextFileToTalks(string path)
        {
            List<Talk> talks = null;
            using (var fin = new FileStream(path, FileMode.Open, FileAccess.Read))
            {
                talks = new List<Talk>();
                var rxName = new Regex(@"^\[(.+?)\]");
                var rxEmptyLine = new Regex(@"^\s*$");

                var castName = "";
                var baseParam = new List<TalkParam>();
                float baseWait = 0.0f;

                var sr = new StreamReader(fin);
                string line;
                while ((line = sr.ReadLine()) != null)
                {
                    var matcheName = rxName.Match(line);
                    if (matcheName.Success)
                    {
                        castName = matcheName.Groups[1].Value;

                        var cast = RemoteTalkProvider.FindCast(castName);
                        if (cast != null)
                            baseParam = TalkParam.Clone(cast.paramInfo).ToList();
                        else
                            baseParam.Clear();
                        TalkParam.Merge(baseParam, ExtractTalkParams(ref line, ref baseWait));
                    }
                    else if (!rxEmptyLine.IsMatch(line))
                    {
                        var param = TalkParam.Clone(baseParam);
                        float wait = -1.0f;
                        TalkParam.Merge(param, ExtractTalkParams(ref line, ref wait, true));
                        if (wait < 0.0f)
                            wait = baseWait;

                        var talk = new Talk();
                        talk.castName = castName;
                        talk.param = param.ToArray();
                        talk.wait = wait;
                        talk.text = line;
                        talks.Add(talk);
                    }
                }
            }
            return talks;
        }

        public static bool TalksToTextFile(string path, IEnumerable<Talk> talks)
        {
            bool ret = false;
            using (var fo = new FileStream(path, FileMode.Create, FileAccess.Write))
            {
                var sb = new StringBuilder();
                foreach (var talk in talks)
                {
                    sb.Append("[" + talk.castName + "]");
                    if (talk.param.Length > 0)
                    {
                        sb.Append(" {");
                        var parray = talk.param.Select(p => p.name + ":" + p.value).ToList();
                        if (talk.wait > 0.0f)
                            parray.Add("wait:" + talk.wait);
                        sb.Append(String.Join(", ", parray.ToArray()));
                        sb.Append("}\r\n");
                    }
                    sb.Append(talk.text);
                    sb.Append("\r\n\r\n");
                }
                byte[] data = new UTF8Encoding(true).GetBytes(sb.ToString());
                fo.Write(data, 0, data.Length);
                ret = true;
            }
            return ret;
        }

        public bool ImportText(string path)
        {
            var talks = TextFileToTalks(path);
            if (talks != null)
            {
                m_talks = talks;
                return true;
            }
            return false;
        }

        public bool ExportText(string path)
        {
            return TalksToTextFile(path, m_talks);
        }

        void UpdateTalk()
        {
            if (!isPlaying || !isActiveAndEnabled)
                return;

#if UNITY_EDITOR
            var delta = (float)(EditorApplication.timeSinceStartup - m_prevTime);
            m_prevTime = EditorApplication.timeSinceStartup;
#else
            var delta = Time.deltaTime;
#endif

            if (m_prevProvider == null || m_prevProvider.isReady)
            {
                if(m_talkPos >= m_talks.Count)
                {
                    isPlaying = false;
                }
                else
                {
                    if (m_current != null && m_wait < m_current.wait)
                    {
                        m_wait += delta;
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
                            m_wait = 0.0f;
                        }
                    }
                }
#if UNITY_EDITOR
                Misc.RefreshWindows();
#endif
            }
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

        void Start()
        {
            if (m_playOnStart)
                Start();
        }

        void Update()
        {
            UpdateTalk();
        }

    }
}
