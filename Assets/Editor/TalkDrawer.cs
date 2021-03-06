using System.Linq;
using UnityEngine;
using UnityEditor;
using IST.RemoteTalk;

namespace IST.RemoteTalkEditor
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
            totalHeight += lineHeight; // fold params
            if (property.FindPropertyRelative("foldParams").boolValue)
                totalHeight += (param.arraySize + 1) * lineHeight; // params + wait

            totalHeight += TextHeight; // text
            totalHeight += 8.0f; // margin
            return totalHeight;
        }

        public override void OnGUI(Rect position, SerializedProperty property, GUIContent label)
        {
            var castNames = RemoteTalkProvider.allCasts.Select(a => a.name).ToList();

            var castName = property.FindPropertyRelative("castName");
            var param = property.FindPropertyRelative("param");
            int castIndex = castNames.FindIndex(a => a == castName.stringValue);
            bool castMissing = false;
            if (castIndex < 0)
            {
                castMissing = true;
                castNames.Add("(Missing) " + castName.stringValue);
                castIndex = castNames.Count - 1;
            }

            using (new EditorGUI.PropertyScope(position, label, property))
            {
                float lineHeight = EditorGUIUtility.singleLineHeight + EditorGUIUtility.standardVerticalSpacing;
                position.height = EditorGUIUtility.singleLineHeight;

                // cast selector
                EditorGUI.BeginChangeCheck();
                if (castMissing)
                    GUI.contentColor = Color.red;
                castIndex = EditorGUI.Popup(position, "Cast", castIndex, castNames.ToArray());
                GUI.contentColor = Color.white;
                if (EditorGUI.EndChangeCheck())
                {
                    var cast = RemoteTalkProvider.FindCast(castNames[castIndex]);
                    if (cast != null)
                    {
                        castName.stringValue = cast.name;
                        param.arraySize = cast.paramInfo.Length;
                        for (int i = 0; i < cast.paramInfo.Length; ++i)
                            TalkParam.Copy(param.GetArrayElementAtIndex(i), cast.paramInfo[i]);
                    }
                }
                position.y += lineHeight;

                // param list
                var foldParams = property.FindPropertyRelative("foldParams");
                foldParams.boolValue = EditorGUI.Foldout(position, foldParams.boolValue, new GUIContent("Params"));
                position.y += lineHeight;
                if (foldParams.boolValue)
                {
                    EditorGUI.indentLevel++;
                    for (int i = 0; i < param.arraySize; ++i)
                    {
                        EditorGUI.PropertyField(position, param.GetArrayElementAtIndex(i));
                        position.y += lineHeight;
                    }
                    EditorGUI.indentLevel--;

                    EditorGUI.PropertyField(position, property.FindPropertyRelative("wait"));
                    position.y += lineHeight;
                }

                // text box
                position.height = TextHeight;
                var text = property.FindPropertyRelative("text");
                var textStyle = EditorStyles.textField;
                textStyle.wordWrap = true;
                text.stringValue = EditorGUI.TextArea(position, text.stringValue, textStyle);
            }
        }
    }
}
