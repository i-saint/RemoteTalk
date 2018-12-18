#if UNITY_EDITOR
using System.Linq;
using UnityEngine;
using UnityEditor;
using UnityEditorInternal;

namespace IST.RemoteTalk
{
    public static class RemoteTalkEditor
    {
        public static void DrawTalkParams(int castID, SerializedProperty talkParamsProp, RemoteTalkClient client)
        {
            var casts = client.casts;
            if (casts.Length == 0)
                return;

            var castNames = casts.Select(a => a.name).ToArray();

            var castProp = talkParamsProp.FindPropertyRelative("cast");
            castProp.intValue = EditorGUILayout.Popup("Cast", castProp.intValue, castNames);

            if (castID >= 0 && castID < casts.Length)
            {
                var castInfo = casts[castID];
                var paramValues = talkParamsProp.FindPropertyRelative("paramValues");
                for (int pi = 0; pi < castInfo.paramInfo.Length; ++pi)
                {
                    var p = paramValues.GetFixedBufferElementAtIndex(pi);
                    EditorGUILayout.PropertyField(p, new GUIContent(castInfo.paramInfo[pi].name));
                }
            }
        }

        public static ReorderableList CreateTalkList(SerializedObject so, SerializedProperty prop)
        {
            var ret = new ReorderableList(so, prop);
            ret.drawHeaderCallback = (rect) =>
            {
                EditorGUI.LabelField(rect, prop.displayName);
            };
            ret.drawElementCallback = (rect, index, isActive, isFocused) =>
            {
                var element = prop.GetArrayElementAtIndex(index);
                rect.height = EditorGUI.GetPropertyHeight(element);
                EditorGUI.PropertyField(rect, element);
            };
            ret.onAddCallback += (list) =>
            {
                prop.arraySize++;
                list.index = prop.arraySize - 1;
            };
            ret.elementHeightCallback = (int i) =>
            {
                var element = prop.GetArrayElementAtIndex(i);
                return EditorGUI.GetPropertyHeight(element);
            };
            return ret;
        }
    }
}
#endif
