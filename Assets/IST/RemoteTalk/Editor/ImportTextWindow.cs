#if UNITY_EDITOR
#if UNITY_2017_1_OR_NEWER
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;
using UnityEditor;
using UnityEditor.Timeline;

namespace IST.RemoteTalk
{
    public class ImportTextWindow : EditorWindow
    {
        public string path;
        RemoteTalkTrack.TextImportOptions options = new RemoteTalkTrack.TextImportOptions();

        [MenuItem("Assets/RemoteTalk/Import Text")]
        public static void Open()
        {
            var path = EditorUtility.OpenFilePanel("Import Text", ".", "txt");
            if (path.Length != 0)
            {
                var window = (ImportTextWindow)EditorWindow.GetWindow(typeof(ImportTextWindow));
                window.titleContent = new GUIContent("Import Text");
                window.path = path;
                window.Show();
            }
        }

        void OnGUI()
        {
            EditorGUI.BeginDisabledGroup(true);
            GUILayout.TextField(path);
            EditorGUI.EndDisabledGroup();
            options.parCastTrack = EditorGUILayout.Toggle("Tracks For Each Cast", options.parCastTrack);
            options.startTime = EditorGUILayout.DoubleField("Start Time", options.startTime);
            options.interval = EditorGUILayout.DoubleField("Interval", options.interval);

            if (GUILayout.Button("Create Timeline Track"))
            {
                RemoteTalkTrack.ImportText(path, options);
                Close();
            }
        }
    }
}
#endif
#endif
