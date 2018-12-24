#if UNITY_EDITOR
#if UNITY_2017_1_OR_NEWER
using UnityEditor;
using UnityEngine;

namespace IST.RemoteTalk
{
    [CustomEditor(typeof(RemoteTalkClip))]
    public class RemoteTalkClipEditor : Editor
    {
        public override void OnInspectorGUI()
        {
            //DrawDefaultInspector();

            var t = target as RemoteTalkClip;
            var so = serializedObject;

            EditorGUILayout.PropertyField(so.FindProperty("talk"), true);

            var talk = t.talk;
            var provider = talk.provider;
            if (provider != null)
            {
                if (provider.isReady)
                {
                    if (GUILayout.Button("Play"))
                        provider.Play(talk);
                }
                else if(provider.isPlaying)
                {
                    if (GUILayout.Button("Stop"))
                        provider.Stop();
                }
            }
            EditorGUILayout.Space();

            EditorGUI.BeginDisabledGroup(true);
            EditorGUILayout.PropertyField(so.FindProperty("audioClip"), true);
            EditorGUI.EndDisabledGroup();

            so.ApplyModifiedProperties();
        }
    }
}
#endif
#endif
