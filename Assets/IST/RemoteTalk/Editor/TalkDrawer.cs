#if UNITY_EDITOR
using System.Linq;
using UnityEngine;
using UnityEditor;

namespace IST.RemoteTalk
{
    [CustomPropertyDrawer(typeof(Talk))]
    public class TalkDrawer : PropertyDrawer
    {
        const float TextHeight = 60;

        public override float GetPropertyHeight(SerializedProperty property, GUIContent label)
        {
            float lineHeight = EditorGUIUtility.singleLineHeight + EditorGUIUtility.standardVerticalSpacing;
            float totalHeight = 0.0f;
            totalHeight += lineHeight; // cast

            var param = property.FindPropertyRelative("param");
            totalHeight += param.arraySize * lineHeight; // params

            totalHeight += TextHeight; // text
            totalHeight += 4.0f; // margin
            return totalHeight;
        }

        public override void OnGUI(Rect position, SerializedProperty property, GUIContent label)
        {
            var castNames = RemoteTalkProvider.allCasts.Select(a => a.name).ToArray();

            var castName = property.FindPropertyRelative("castName");
            var param = property.FindPropertyRelative("param");

            int ci = 0;
            for (int i = 0; i < castNames.Length; ++i)
            {
                if (castNames[i]== castName.stringValue)
                {
                    ci = i;
                    break;
                }
            }

            using (new EditorGUI.PropertyScope(position, label, property))
            {
                float lineHeight = EditorGUIUtility.singleLineHeight + EditorGUIUtility.standardVerticalSpacing;
                position.height = EditorGUIUtility.singleLineHeight;
                EditorGUI.BeginChangeCheck();
                ci = EditorGUI.Popup(position, "Cast", ci, castNames);
                if (EditorGUI.EndChangeCheck())
                {
                    var cas = RemoteTalkProvider.FindCast(castNames[ci]);
                    castName.stringValue = cas.name;
                    param.arraySize = cas.paramNames.Length;
                    for (int i = 0; i < cas.paramNames.Length; ++i)
                    {
                        var e = param.GetArrayElementAtIndex(i);
                        e.FindPropertyRelative("name").stringValue = cas.paramNames[i];
                    }
                }
                position.y += lineHeight;

                EditorGUI.indentLevel++;
                for (int i = 0; i < param.arraySize; ++i)
                {
                    var e = param.GetArrayElementAtIndex(i);
                    var name = e.FindPropertyRelative("name");
                    var val = e.FindPropertyRelative("value");

                    val.floatValue = EditorGUI.FloatField(position, name.stringValue, val.floatValue);
                    position.y += lineHeight;
                }
                EditorGUI.indentLevel--;

                position.height = TextHeight;
                var text = property.FindPropertyRelative("text");
                text.stringValue = EditorGUI.TextArea(position, text.stringValue);

            }
        }
    }
}
#endif
