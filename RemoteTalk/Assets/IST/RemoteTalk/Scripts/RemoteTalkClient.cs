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

        rtHTTPClient m_client;
        rtTalkParams m_serverParams;
        AvatorInfo[] m_avators = new AvatorInfo[0] { };
        #endregion


        #region Properties
        public string server
        {
            get { return m_server; }
            set { m_server = value; m_client.Release(); }
        }
        public int port
        {
            get { return m_port; }
            set { m_port = value; m_client.Release(); }
        }
        public rtTalkParams talkParams
        {
            get { return m_params; }
            set { m_params = value; }
        }
        public rtTalkParams serverParams { get { return m_serverParams; } }
        public AvatorInfo[] avators { get { return m_avators; } }

        public int sampleLength { get { return m_client.buffer.sampleLength; } }
        public bool isFinished { get { return m_client.isFinished; } }
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

        public void Talk()
        {
            if (!m_client)
                m_client = rtHTTPClient.Create(m_server, m_port);
            m_client.Talk(ref m_params, m_text);
        }

        public void UpdateSamples()
        {
            m_client.SyncBuffers();
        }



        void OnValidate()
        {
            m_client.Release();
        }

        void Awake()
        {

        }

        void OnDestroy()
        {
            m_client.Release();
        }

        void Update()
        {
            UpdateSamples();
        }
    }
}
