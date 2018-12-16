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

            EditorGUI.BeginChangeCheck();
            EditorGUILayout.DelayedTextField(so.FindProperty("m_serverAddress"));
            EditorGUILayout.DelayedIntField(so.FindProperty("m_serverPort"));
            if (EditorGUI.EndChangeCheck())
            {
                so.ApplyModifiedProperties();
                t.RefreshClient();
            }
            if (GUILayout.Button("Refresh"))
                t.RefreshClient();

            EditorGUILayout.Space();

            EditorGUILayout.PropertyField(so.FindProperty("m_talkAudio"), true);

            EditorGUILayout.Space();

            EditorGUILayout.LabelField("Host: " + t.host);

            if(t.isServerReady)
            {
                var talkParams = so.FindProperty("m_talkParams");
                RemoteTalkEditor.DrawTalkParams(t.castID, talkParams, t);

                var text = so.FindProperty("m_talkText");
                EditorGUI.BeginChangeCheck();
                text.stringValue = EditorGUILayout.TextArea(text.stringValue, GUILayout.Height(100));
                if (EditorGUI.EndChangeCheck())
                    so.ApplyModifiedProperties();

                if (t.isIdling)
                {
                    if (GUILayout.Button("Talk"))
                        t.Talk();
                }
                else
                {
                    if (GUILayout.Button("Stop"))
                        t.Stop();
                }

                EditorGUILayout.Space();

                var exportAudio = so.FindProperty("m_exportAudio");
                EditorGUILayout.PropertyField(exportAudio);
                if (exportAudio.boolValue)
                {
                    EditorGUILayout.PropertyField(so.FindProperty("m_exportDir"));

                    var exportFileFormat = so.FindProperty("m_exportFileFormat");
                    EditorGUILayout.PropertyField(exportFileFormat);
                    if (exportFileFormat.intValue == (int)rtFileFormat.Ogg)
                        EditorGUILayout.PropertyField(so.FindProperty("m_oggSettings"), true);
                }

                EditorGUILayout.PropertyField(so.FindProperty("m_useCache"));
                EditorGUILayout.PropertyField(so.FindProperty("m_sampleGranularity"));
                EditorGUILayout.PropertyField(so.FindProperty("m_logging"));
            }

            if (GUI.changed)
                so.ApplyModifiedProperties();

            EditorGUILayout.Space();

            if (GUILayout.Button("Connect VOICEROID2"))
                rtPlugin.LaunchVOICEROID2();
            EditorGUILayout.Space();
            if (GUILayout.Button("Connect CeVIO CS"))
                rtPlugin.LaunchCeVIOCS();

            EditorGUILayout.Space();

            if (GUILayout.Button("Start SAPI Server"))
                rtspTalkServer.StartServer();
        }
    }
}
#endif
