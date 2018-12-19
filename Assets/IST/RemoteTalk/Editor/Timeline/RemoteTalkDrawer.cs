using UnityEditor;
using UnityEngine;

namespace IST.RemoteTalk
{
    [CustomPropertyDrawer(typeof(RemoteTalkBehaviour))]
    public class RemoteTalkDrawer : PropertyDrawer
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
