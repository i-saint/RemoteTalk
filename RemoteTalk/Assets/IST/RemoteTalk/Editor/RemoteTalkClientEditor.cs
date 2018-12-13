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
            //DrawDefaultInspector();
            //EditorGUILayout.Space();

            var t = target as RemoteTalkClient;
            var so = serializedObject;

            EditorGUILayout.PropertyField(so.FindProperty("m_serverAddress"));
            EditorGUILayout.PropertyField(so.FindProperty("m_serverPort"));

            EditorGUILayout.Space();

            var exportAudio = so.FindProperty("m_exportAudio");
            EditorGUILayout.PropertyField(exportAudio);
            if (exportAudio.boolValue)
                EditorGUILayout.PropertyField(so.FindProperty("m_exportDir"));
            EditorGUILayout.PropertyField(so.FindProperty("m_sampleGranularity"));
            EditorGUILayout.PropertyField(so.FindProperty("m_logging"));

            EditorGUILayout.Space();

            EditorGUILayout.LabelField("Host: " + t.host);

            var talkParams = so.FindProperty("m_talkParams");
            RemoteTalkEditor.DrawTalkParams(t.castID, talkParams, t);
            EditorGUILayout.TextArea(t.talkText, GUILayout.Height(100));

            if (GUI.changed)
                so.ApplyModifiedProperties();

            if (GUILayout.Button("Talk"))
                t.Talk();

            EditorGUILayout.Space();

            //if (GUILayout.Button("Start SAPI Server"))
            //    rtspTalkServer.StartServer();
        }
    }
}
#endif
