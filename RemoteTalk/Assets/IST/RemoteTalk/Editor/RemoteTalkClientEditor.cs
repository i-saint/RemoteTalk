#if UNITY_EDITOR
using UnityEngine;
using UnityEditor;

namespace IST.RemoteTalk
{
    [CustomEditor(typeof(RemoteTalkClient))]
    public class RemoteTalkClientEditor : Editor
    {
        public override void OnInspectorGUI()
        {
            DrawDefaultInspector();
            EditorGUILayout.Space();

            var t = target as RemoteTalkClient;
            if (GUILayout.Button("Talk"))
                t.Talk();

            if (GUILayout.Button("Start SAPI Server"))
            {
                rtspTalkServer.StartServer();
            }
        }
    }
}
#endif
