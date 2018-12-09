using System;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace IST.RemoteTalk
{
    [ExecuteInEditMode]
    public class RemoteTalkClient : MonoBehaviour
    {
        #region Fields
        [SerializeField] string m_server = "localhost";
        [SerializeField] int m_port = 8081;
        [Space(10)]
        [SerializeField] rtTalkParams m_params = rtTalkParams.defaultValue;
        [SerializeField] string m_text;
        [SerializeField] string m_avatorName = "";

        [SerializeField] bool m_exportAudio = false;
        [SerializeField] string m_assetDir = "RemoteTalkAssets";

        rtHTTPClient m_client;
        rtAsync m_asyncStat;
        rtAsync m_asyncTalk;
        rtAsync m_asyncStop;
        rtTalkParams m_serverParams;
        AvatorInfo[] m_avators = new AvatorInfo[0] { };

        int m_samplePos;
        bool m_readySamples = false;
        bool m_logging = true;
        bool m_ready = false;
        bool m_talking = false;
        int m_bufferDepleted = 0;
        #endregion


        #region Properties
        public string server
        {
            get { return m_server; }
            set { m_server = value; ReleaseClient(); }
        }
        public int port
        {
            get { return m_port; }
            set { m_port = value; ReleaseClient(); }
        }
        public rtTalkParams talkParams
        {
            get { return m_params; }
            set { m_params = value; }
        }
        public string talkText
        {
            get { return m_text; }
            set { m_text = value; }
        }

        public bool isReady
        {
            get { return m_ready; }
        }
        public bool isTalking
        {
            get { return m_talking; }
        }
        public bool isPlaying
        {
            get
            {
                var source = GetComponent<AudioSource>();
                return source != null && source.isPlaying;
            }
        }


        public rtTalkParams serverParams { get { return m_serverParams; } }
        public AvatorInfo[] avators { get { return m_avators; } }

        public int sampleLength { get { return m_client.buffer.sampleLength; } }
        public string assetPath { get { return "Assets/" + m_assetDir; } }
        #endregion


#if UNITY_EDITOR
        [MenuItem("GameObject/RemoteTalk/Create Client", false, 10)]
        public static void CreateRemoteTalkClient(MenuCommand menuCommand)
        {
            var go = new GameObject();
            go.name = "RemoteTalkClient";
            go.AddComponent<AudioSource>();
            go.AddComponent<RemoteTalkClient>();
            Undo.RegisterCreatedObjectUndo(go, "RemoteTalk");
        }
#endif

        void MakeClient()
        {
            if (!m_client)
            {
                m_client = rtHTTPClient.Create(m_server, m_port);
                m_asyncStat = m_client.UpdateServerStatus();
            }
        }

        void ReleaseClient()
        {
            m_asyncStat.Release();
            m_asyncTalk.Release();
            m_asyncStop.Release();
            m_client.Release();
        }


        public void Talk()
        {
            MakeClient();

            if (m_avators.Length > 0)
            {
                m_params.avator = Mathf.Clamp(m_params.avator, 0, m_avators.Length - 1);
                m_avatorName = m_avators[m_params.avator].name;
            }
            m_params.flags = m_serverParams.flags;
            m_params.mute = true;
            m_readySamples = false;
            m_talking = true;
            m_asyncTalk = m_client.Talk(ref m_params, m_text);
        }

        public void Stop()
        {
            MakeClient();
            m_asyncStop = m_client.Talk(ref m_params, m_text);
        }

        void OnAudioRead(float[] samples)
        {
            if (!m_readySamples)
            {
                rtAudioData.ClearSamples(samples);
                return;
            }

            for (int i = 0; i < 20; ++i)
            {
                if (m_samplePos + samples.Length <= m_client.SyncBuffers().sampleLength || m_asyncTalk.isFinished)
                    break;
                System.Threading.Thread.Sleep(16);
            }

            //Debug.Log("read pos: " + m_samplePos + " [" + samples.Length + "]");
            m_samplePos += m_client.SyncBuffers().ReadSamples(samples, m_samplePos, samples.Length);
        }

        void OnAudioSetPosition(int pos)
        {
            if (!m_talking && m_samplePos >= m_client.buffer.sampleLength)
                ++m_bufferDepleted;
        }

        public void UpdateSamples()
        {
            if (m_asyncStat && m_asyncStat.isFinished)
            {
                m_serverParams = m_client.serverParams;
                m_avators = m_client.avatorList;
                m_asyncStat.Release();
                m_ready = true;
#if UNITY_EDITOR
                Misc.ForceRepaint();
#endif
            }

            if (m_asyncTalk)
            {
                var buf = m_client.SyncBuffers();
                if (buf.sampleLength > 0)
                {
                    var source = GetComponent<AudioSource>();
                    if (source != null && !source.isPlaying)
                    {
                        source.clip = AudioClip.Create("RemoteTalkAudio",
                            buf.frequency * buf.channels / 5, // 200ms
                            buf.channels, buf.frequency, true,
                            OnAudioRead, OnAudioSetPosition);
                        source.loop = true;
                        source.Play();

                        // must be after Play()
                        m_samplePos = 0;
                        m_bufferDepleted = 0;
                        m_readySamples = true;
                    }
                }


                if (m_asyncTalk.isFinished)
                {
                    m_asyncTalk.Release();
                    m_talking = false;

#if UNITY_EDITOR
                    if (m_exportAudio)
                    {
                        MakeSureAssetDirectoryExists();
                        var dstPath = assetPath + "/" + "test.wav";
                        if (m_client.buffer.ExportAsWave(dstPath))
                        {
                            AssetDatabase.ImportAsset(dstPath);
                            AudioClip ac = AssetDatabase.LoadAssetAtPath<AudioClip>(dstPath);
                            if (ac != null)
                            {
                                var importer = (AudioImporter)AssetImporter.GetAtPath(dstPath);
                                if (importer != null)
                                {
                                    // nothing todo for now
                                }
                            }
                        }
                    }
                }
#endif
            }

            if (!m_talking)
            {
                var source = GetComponent<AudioSource>();
                if (source != null && source.isPlaying)
                {
                    if (m_bufferDepleted > 2 && source.loop)
                        source.loop = false;
                }
            }
        }

#if UNITY_EDITOR
        bool Try(Action act)
        {
            try
            {
                act.Invoke();
                return true;
            }
            catch (Exception e)
            {
                if (m_logging)
                    Debug.LogError(e);
                return false;
            }
        }

        public void MakeSureAssetDirectoryExists()
        {
            Try(() =>
            {
                if (!AssetDatabase.IsValidFolder(assetPath))
                    AssetDatabase.CreateFolder("Assets", m_assetDir);
            });
        }
#endif



        void OnValidate()
        {
            ReleaseClient();
        }

        void OnEnable()
        {
#if UNITY_EDITOR
            EditorApplication.update += UpdateSamples;
#endif
        }

        void OnDisable()
        {
#if UNITY_EDITOR
            EditorApplication.update -= UpdateSamples;
#endif
            ReleaseClient();
        }

        void Update()
        {
            MakeClient();
            UpdateSamples();
        }
    }
}
