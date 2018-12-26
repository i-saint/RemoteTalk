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
                t.ImportText(EditorUtility.OpenFilePanel("Import Text", ".", "txt"));
            if (GUILayout.Button("Export Text"))
                t.ExportText(EditorUtility.SaveFilePanel("Export Text", ".", t.name, "txt"));
            EditorGUILayout.EndHorizontal();

            EditorGUILayout.Space();
            if(GUILayout.Button("Convet To AudioTrack"))
                t.ConvertToAudioTrack();
        }
    }

}
#endif
#endif
