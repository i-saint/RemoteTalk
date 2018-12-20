using UnityEditor;
using UnityEngine;

namespace IST.RemoteTalk
{
    [CustomEditor(typeof(RemoteTalkTrack))]
    public class RemoteTalkTrackEditor : Editor
    {
        public override void OnInspectorGUI()
        {
            DrawDefaultInspector();

            var t = target as RemoteTalkTrack;

            EditorGUILayout.Space();
            if(GUILayout.Button("Convet To AudioTrack"))
            {
                t.ConvertToAudioTrack();
            }
        }
    }

}
