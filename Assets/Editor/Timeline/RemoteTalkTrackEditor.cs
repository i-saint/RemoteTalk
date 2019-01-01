#if UNITY_2017_1_OR_NEWER
using UnityEngine;
using UnityEditor;
using IST.RemoteTalk;

namespace IST.RemoteTalkEditor
{
    [CustomEditor(typeof(RemoteTalkTrack)), CanEditMultipleObjects]
    public class RemoteTalkTrackEditor : Editor
    {
        public override void OnInspectorGUI()
        {
            DrawDefaultInspector();
            DrawRemoteTalkTrackSettings();
        }

        public static void DrawRemoteTalkTrackSettings()
        {
            EditorGUI.BeginChangeCheck();
            var pauseWhenExport = EditorGUILayout.Toggle("Pause When Export", RemoteTalkTrack.pauseWhenExport);
            if (EditorGUI.EndChangeCheck())
                RemoteTalkTrack.pauseWhenExport = pauseWhenExport;

            EditorGUI.BeginChangeCheck();
            var fitDuration = EditorGUILayout.Toggle("Fit Duration", RemoteTalkTrack.fitDuration);
            if (EditorGUI.EndChangeCheck())
                RemoteTalkTrack.fitDuration = fitDuration;

            if (fitDuration)
            {
                EditorGUI.indentLevel++;
                EditorGUI.BeginChangeCheck();
                var arrangeScope = (RemoteTalkTrack.ArrangeScope)EditorGUILayout.EnumPopup("Arrange Clips", RemoteTalkTrack.arrangeClips);
                if (EditorGUI.EndChangeCheck())
                    RemoteTalkTrack.arrangeClips = arrangeScope;
                EditorGUI.indentLevel--;
            }

            EditorGUILayout.Space();

            EditorGUILayout.BeginHorizontal();
            if (GUILayout.Button("Import Text"))
            {
                ImportTextWindow.Open();
            }
            if (GUILayout.Button("Export Text"))
            {
                var filename = "RemoteTalkTimeline";
                if (Misc.currentTimeline)
                    filename = Misc.currentTimeline.name;
                RemoteTalkTrack.ExportText(EditorUtility.SaveFilePanel("Export Text", ".", filename, "txt"));
            }
            EditorGUILayout.EndHorizontal();

            EditorGUILayout.Space();
            if (GUILayout.Button("Convet To AudioTrack"))
            {
                RemoteTalkTrack.ConvertToAudioTrack();
                Misc.RefreshTimelineWindow();
            }
        }
    }

}
#endif
