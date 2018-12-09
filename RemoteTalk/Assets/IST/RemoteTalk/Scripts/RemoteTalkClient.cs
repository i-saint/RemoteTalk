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

        [SerializeField] rtTalkParams m_serverParams;
        [SerializeField] AvatorInfo[] m_avators = new AvatorInfo[0] { };

        rtHTTPClient m_client;
        rtAsync m_asyncStat;
        rtAsync m_asyncTalk;
        rtAsync m_asyncStop;
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
        public rtTalkParams serverParams { get { return m_serverParams; } }
        public AvatorInfo[] avators { get { return m_avators; } }

        public int sampleLength { get { return m_client.buffer.sampleLength; } }
        #endregion


#if UNITY_EDITOR
        [MenuItem("GameObject/RemoteTalk/Create Client", false, 10)]
        public static void CreateRemoteTalkClient(MenuCommand menuCommand)
        {
            var go = new GameObject();
            go.name = "RemoteTalkClient";
            var mss = go.AddComponent<RemoteTalkClient>();
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
            m_asyncTalk = m_client.Talk(ref m_params, m_text);
        }

        public void Stop()
        {
            MakeClient();
            m_asyncStop = m_client.Talk(ref m_params, m_text);
        }

        public void UpdateSamples()
        {
            if (m_asyncStat && m_asyncStat.isFinished)
            {
                m_serverParams = m_client.serverParams;
                m_avators = m_client.avatorList;
                m_asyncStat.Release();
#if UNITY_EDITOR
                Misc.ForceRepaint();
#endif
            }

            if (m_asyncTalk)
            {
                m_client.SyncBuffers();
                if (m_asyncTalk.isFinished)
                    m_asyncTalk.Release();
            }
        }



        void OnValidate()
        {
            ReleaseClient();
        }

        void Awake()
        {
#if UNITY_EDITOR
            EditorApplication.update += UpdateSamples;
#endif
        }

        void OnDestroy()
        {
#if UNITY_EDITOR
            EditorApplication.update -= UpdateSamples;
#endif
            ReleaseClient();
        }

        void Update()
        {
            UpdateSamples();
        }
    }
}
