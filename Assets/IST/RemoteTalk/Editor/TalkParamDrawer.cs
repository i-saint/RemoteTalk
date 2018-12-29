using System.Linq;
using UnityEngine;
using UnityEditor;
using IST.RemoteTalk;

namespace IST.RemoteTalkEditor
{
    [CustomPropertyDrawer(typeof(TalkParam))]
    public class TalkParamDrawer : PropertyDrawer
    {
        const float TextHeight = 60;

        public override void OnGUI(Rect position, SerializedProperty property, GUIContent label)
        {
            var name = property.FindPropertyRelative("name");
            var val = property.FindPropertyRelative("value");
            var rmin = property.FindPropertyRelative("rangeMin").floatValue;
            var rmax = property.FindPropertyRelative("rangeMax").floatValue;

            EditorGUI.BeginChangeCheck();
            float v = 0.0f;
            if (rmax > rmin)
                v = EditorGUI.Slider(position, name.stringValue, val.floatValue, rmin, rmax);
            else
                v = EditorGUI.FloatField(position, name.stringValue, val.floatValue);
            if (EditorGUI.EndChangeCheck())
                val.floatValue = v;
        }
    }
}
