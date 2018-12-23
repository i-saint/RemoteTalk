#if UNITY_EDITOR
#if UNITY_2017_1_OR_NEWER
using UnityEditor;
using UnityEngine;

namespace IST.RemoteTalk
{
    [CustomPropertyDrawer(typeof(RemoteTalkBehaviour))]
    public class RemoteTalkBehaviourDrawer : PropertyDrawer
    {
        public override float GetPropertyHeight(SerializedProperty property, GUIContent label)
        {
            return 0.0f;
        }

        public override void OnGUI(Rect position, SerializedProperty property, GUIContent label)
        {
        }
    }
}
#endif
#endif
