#if UNITY_2017_1_OR_NEWER
using UnityEngine;
using UnityEditor;
using IST.RemoteTalk;

namespace IST.RemoteTalkEditor
{
    public class ImportTextWindow : EditorWindow
    {
        public string path;
        RemoteTalkTrack.TextImportOptions options = new RemoteTalkTrack.TextImportOptions();

        [MenuItem("GameObject/Remote Talk/Create Timeline From Text File")]
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
