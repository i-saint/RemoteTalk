#if UNITY_EDITOR
using System.Linq;
using UnityEngine;
using UnityEditor;
using UnityEditorInternal;

namespace IST.RemoteTalk
{
    [CustomEditor(typeof(RemoteTalkScript))]
    public class RemoteTalkScriptEditor : Editor
    {
        ReorderableList m_rlist;

        void OnEnable()
        {
            m_rlist = RemoteTalkEditor.CreateTalkList(serializedObject, serializedObject.FindProperty("m_talks"));
        }

        public override void OnInspectorGUI()
        {
            //DrawDefaultInspector();


            serializedObject.Update();
            m_rlist.DoLayoutList();
            serializedObject.ApplyModifiedProperties();


            var t = target as RemoteTalkScript;
            var so = serializedObject;

            EditorGUILayout.Space();

            if(t.isPlaying)
            {
                if (GUILayout.Button("Stop"))
                    t.Stop();
            }
            else
            {
                if (GUILayout.Button("Play"))
                    t.Play();
            }
        }
    }
}
#endif
