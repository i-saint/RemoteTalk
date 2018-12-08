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
        [SerializeField] int m_avatorID = 0;
        [SerializeField] string m_avatorName = "";
        [Space(5)]
        [Space(5)]
        [SerializeField] float m_volume = 1.0f;
        [SerializeField] float m_speed = 1.0f;
        [SerializeField] float m_pitch = 1.0f;
        [SerializeField] float m_intonation = 1.0f;
        [Space(5)]
        [SerializeField] float m_joy = 0.0f;
        [SerializeField] float m_anger = 0.0f;
        [SerializeField] float m_sorrow = 0.0f;
        [Space(10)]
        [SerializeField] string m_text;

        rtHTTPClient m_client;
        rtTalkParams m_serverParams;
        AvatorInfo[] m_avators = new AvatorInfo[0] { };
        #endregion


        #region Properties
        public float volume
        {
            get { return m_volume; }
            set { m_volume = value; }
        }
        public float speed
        {
            get { return m_speed; }
            set { m_speed = value; }
        }
        public float pitch
        {
            get { return m_pitch; }
            set { m_pitch = value; }
        }
        public float intonation
        {
            get { return m_intonation; }
            set { m_intonation = value; }
        }
        public float joy
        {
            get { return m_joy; }
            set { m_joy = value; }
        }
        public float anger
        {
            get { return m_anger; }
            set { m_anger = value; }
        }
        public float sorrow
        {
            get { return m_sorrow; }
            set { m_sorrow = value; }
        }
        public int avatorID
        {
            get { return m_avatorID; }
            set { m_avatorID = value; }
        }

        public rtTalkParams serverParams { get { return m_serverParams; } }
        public AvatorInfo[] avators { get { return m_avators; } }
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

        void OnEnable()
        {
            
        }

        void OnDisable()
        {
            
        }
    }
}
