#if UNITY_EDITOR
using System.Linq;
using UnityEngine;
using UnityEditor;

namespace IST.RemoteTalk
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

            if (rmax > rmin)
                val.floatValue = EditorGUI.Slider(position, name.stringValue, val.floatValue, rmin, rmax);
            else
                val.floatValue = EditorGUI.FloatField(position, name.stringValue, val.floatValue);
        }
    }
}
#endif
