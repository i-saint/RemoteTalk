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

        static List<TalkParam> ExtractTalkParams(ref string line, bool clearParamBlock = false)
        {
            var rxParamBlock = new Regex(@"\{([^ }]+)\}\s*", RegexOptions.Compiled);
            var rxParam = new Regex(@"([^ ]+?)\s*:\s*([\d.]+)", RegexOptions.Compiled);

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
                        ret.Add(new TalkParam
                        {
                            name = matcheParam.Groups[1].Value,
                            value = float.Parse(matcheParam.Groups[2].Value),
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
                var rxName = new Regex(@"^\[(.+?)\]", RegexOptions.Compiled);
                var rxEmptyLine = new Regex(@"^\s*$", RegexOptions.Compiled);

                var castName = "";
                var baseParam = new List<TalkParam>();

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
                        TalkParam.Merge(baseParam, ExtractTalkParams(ref line));
                    }
                    else if (!rxEmptyLine.IsMatch(line))
                    {
                        var param = TalkParam.Clone(baseParam);
                        TalkParam.Merge(param, ExtractTalkParams(ref line, true));

                        var talk = new Talk();
                        talk.castName = castName;
                        talk.text = line;
                        talk.param = param.ToArray();
                        talks.Add(talk);
                    }
                }
            }
            return talks;
        }

        public static bool TalksToTextFile(string path, List<Talk> talks)
        {
            bool ret = false;
            using (var fo = new FileStream(path, FileMode.Create, FileAccess.Write))
            {
                var sb = new StringBuilder();
                foreach (var t in talks)
                {
                    sb.Append("[" + t.castName + "]");
                    if (t.param.Length > 0)
                    {
                        sb.Append(" {");
                        sb.Append(String.Join(", ", t.param.Select(p => p.name + ":" + p.value)));
                        sb.Append("}\r\n");
                    }
                    sb.Append(t.text);
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

#if UNITY_2017_1_OR_NEWER
        public bool ConvertToRemoteTalkTrack()
        {
            var director = Misc.GetOrAddComponent<PlayableDirector>(gameObject);
            if (director == null)
                return false;

            var timeline = director.playableAsset as TimelineAsset;
            if (timeline == null)
                return false;

            var track = timeline.CreateTrack<RemoteTalkTrack>(null, name);
            double timeOffset = 0.0;
            foreach (var talk in m_talks)
            {
                var dstClip = track.AddClip(talk);
                dstClip.start = timeOffset;
                timeOffset += dstClip.duration;
            }
            Misc.RefreshTimelineWindow();

            return true;
        }
#endif

        void UpdateTalk()
        {
            if (!m_isPlaying || !isActiveAndEnabled)
                return;

            if (m_prevProvider == null || m_prevProvider.isReady)
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
