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
            DrawDefaultInspector();
        }
    }
}
#endif
