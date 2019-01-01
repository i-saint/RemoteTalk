#if UNITY_2017_1_OR_NEWER
using System.Linq;
using UnityEditor;
using UnityEngine;
using IST.RemoteTalk;

namespace IST.RemoteTalkEditor
{
    [CustomEditor(typeof(RemoteTalkClip)), CanEditMultipleObjects]
    public class RemoteTalkClipEditor : Editor
    {
        public override void OnInspectorGUI()
        {
            //DrawDefaultInspector();

            var so = serializedObject;

            // TalkDrawer doesn't work well with multiple objects
            //EditorGUILayout.PropertyField(so.FindProperty("talk"), true);

            var castNames = RemoteTalkProvider.allCasts.Select(a => a.name).ToList();

            var castName = so.FindProperty("talk.castName");
            var param = so.FindProperty("talk.param");
            int castIndex = castNames.FindIndex(a => a == castName.stringValue);
            bool castMissing = false;
            if (castIndex < 0)
            {
                castMissing = true;
                castNames.Add("(Missing) " + castName.stringValue);
                castIndex = castNames.Count - 1;
            }

            // cast selector
            EditorGUI.BeginChangeCheck();
            if (!castName.hasMultipleDifferentValues && castMissing)
                GUI.contentColor = Color.red;
            EditorGUI.showMixedValue = castName.hasMultipleDifferentValues;
            castIndex = EditorGUILayout.Popup("Cast", castIndex, castNames.ToArray());
            EditorGUI.showMixedValue = false;
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

            // param list
            {
                EditorGUI.indentLevel++;
                for (int i = 0; i < param.arraySize; ++i)
                {
                    var p = param.GetArrayElementAtIndex(i);
                    var name = p.FindPropertyRelative("name");
                    if (name != null && !name.hasMultipleDifferentValues)
                    {
                        var val = p.FindPropertyRelative("value");
                        var rmin = p.FindPropertyRelative("rangeMin").floatValue;
                        var rmax = p.FindPropertyRelative("rangeMax").floatValue;

                        EditorGUI.BeginChangeCheck();
                        EditorGUI.showMixedValue = val.hasMultipleDifferentValues;
                        float v = 0.0f;
                        if (rmax > rmin)
                            v = EditorGUILayout.Slider(name.stringValue, val.floatValue, rmin, rmax);
                        else
                            v = EditorGUILayout.FloatField(name.stringValue, val.floatValue);
                        EditorGUI.showMixedValue = false;
                        if (EditorGUI.EndChangeCheck())
                            val.floatValue = v;

                        GUI.contentColor = Color.white;
                    }
                }
                EditorGUI.indentLevel--;
            }

            // text box
            var text = so.FindProperty("talk.text");
            var textStyle = EditorStyles.textField;
            textStyle.wordWrap = true;

            EditorGUI.showMixedValue = text.hasMultipleDifferentValues;
            text.stringValue = EditorGUILayout.TextArea(text.stringValue, textStyle, GUILayout.Height(100));
            EditorGUI.showMixedValue = false;

            EditorGUI.BeginDisabledGroup(true);
            EditorGUILayout.PropertyField(so.FindProperty("audioClip"), true);
            EditorGUI.EndDisabledGroup();

            so.ApplyModifiedProperties();

            EditorGUILayout.Space();
            DrawHorizontalLine();

            RemoteTalkTrack.fold = EditorGUILayout.Foldout(RemoteTalkTrack.fold, "Track Settings");
            if (RemoteTalkTrack.fold)
                RemoteTalkTrackEditor.DrawRemoteTalkTrackSettings();
        }

        void DrawHorizontalLine(int thickness = 1)
        {
            Rect rect = EditorGUILayout.GetControlRect(false, thickness);
            rect.height = thickness;
            EditorGUI.DrawRect(rect, Color.gray);
        }
    }
}
#endif
