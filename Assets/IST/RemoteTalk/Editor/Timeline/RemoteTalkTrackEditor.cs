#if UNITY_EDITOR
#if UNITY_2017_1_OR_NEWER
using UnityEditor;
using UnityEngine;

namespace IST.RemoteTalk
{
    [CustomEditor(typeof(RemoteTalkTrack)), CanEditMultipleObjects]
    public class RemoteTalkTrackEditor : Editor
    {
        public override void OnInspectorGUI()
        {
            DrawDefaultInspector();

            var t = target as RemoteTalkTrack;

            EditorGUILayout.Space();

            EditorGUILayout.BeginHorizontal();
            if (GUILayout.Button("Import Text"))
            {
                ImportTextWindow.Open();
            }
            if (GUILayout.Button("Export Text"))
            {
                RemoteTalkTrack.ExportText(EditorUtility.SaveFilePanel("Export Text", ".", t.timelineAsset.name, "txt"));
            }
            EditorGUILayout.EndHorizontal();

            EditorGUILayout.Space();
            if(GUILayout.Button("Convet To AudioTrack"))
            {
                RemoteTalkTrack.ConvertToAudioTrack();
                Misc.RefreshTimelineWindow();
            }
        }
    }

}
#endif
#endif
