using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace IST.RemoteTalk
{
    [ExecuteInEditMode]
    public class RemoteTalkReceiver : MonoBehaviour
    {
        #region Fields
        [SerializeField] int m_port = 8090;
        #endregion

#if UNITY_EDITOR
        [MenuItem("GameObject/RemoteTalk/Create Receiver", false, 10)]
        public static void CreateRemoteTalkReceiver(MenuCommand menuCommand)
        {
            var go = new GameObject();
            go.name = "RemoteTalkReceiver";
            go.AddComponent<AudioSource>();
            go.AddComponent<RemoteTalkReceiver>();
            Undo.RegisterCreatedObjectUndo(go, "RemoteTalk");
        }
#endif
    }
}
