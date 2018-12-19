using UnityEditor;
using UnityEngine;

namespace IST.RemoteTalk
{
    [CustomPropertyDrawer(typeof(RemoteTalkBehaviour))]
    public class RemoteTalkDrawer : PropertyDrawer
    {
        public override float GetPropertyHeight(SerializedProperty property, GUIContent label)
        {
            int fieldCount = 4;
            return fieldCount * EditorGUIUtility.singleLineHeight;
        }

        public override void OnGUI(Rect position, SerializedProperty property, GUIContent label)
        {
            Rect singleFieldRect = new Rect(position.x, position.y, position.width, EditorGUIUtility.singleLineHeight);
            EditorGUI.LabelField(singleFieldRect, "Test");
        }
    }
}
