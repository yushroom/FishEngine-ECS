using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using UnityEngine.SceneManagement;
//using System.Runtime.Serialization.Json;
//using UnityEngine;
using System;

namespace FishEngine
{
    [System.Serializable]
    class Model
    {
        public Asset asset = new Asset();
        public string name;
        public List<Node> nodes = new List<Node>();
        public Mesh[] meshes;
        public byte[] bin;

        public Node CreateNode(Transform t)
        {
            var go = t.gameObject;
            Node node = new Node(t.gameObject);
            nodes.Add(node);
            node.id = nodes.Count - 1;
            int count = go.transform.childCount;
            if (count != 0)
            {
                node.children = new int[count];
                for (int i = 0; i < go.transform.childCount; ++i)
                {
                    var c = go.transform.GetChild(i);
                    var child = CreateNode(c);
                    node.children[i] = child.id;
                }
            }
            return node;
        }
    }


    [Serializable]
    class Asset
    {
        public string version = "2.0";
        public string generator = "FishEngine-Unity2glTF";
    }

    [Serializable]
    class Mesh
    {
        public string name;
        public Primitive[] primitives;
    }

    [Serializable]
    class Primitive
    {
        public Dictionary<string, int> attributes = new Dictionary<string, int>();
    }

    [Serializable]
    class Node
    {
        [NonSerialized]
        public int id;

        [NonSerialized]
        Transform transform;

        public string name;
        public int mesh;
        public Vector3 position;
        public Vector3 scale;
        public float[] rotation;
        //public float[] matrix;

        public int[] children;

        //[NonSerialized]
        //public Node[] children;

        public Node(GameObject go)
        {
            name = go.name;
            transform = go.transform;
            rotation = new float[4];
            Quaternion rot = go.transform.localRotation;
            rotation[0] = rot.x;
            rotation[1] = rot.y;
            rotation[2] = rot.z;
            rotation[3] = rot.w;
            position = go.transform.localPosition;
            scale = go.transform.localScale;
        }
    }

    

    public class Unity2glTF : MonoBehaviour
    {
        [MenuItem("FishEngine/Convert to glTF")]
        static void Convert()
        {
            Model model = new Model();
            var scene = SceneManager.GetActiveScene();

            model.name = scene.name;

            var gos = scene.GetRootGameObjects();
            //gos[0].GetComponentsInChildren<Transform>();
            //model.nodes = new Node[gos.Length];

            List<UnityEngine.Mesh> meshes = new List<UnityEngine.Mesh>();
            foreach (var go in gos)
            {
                var mfs = go.GetComponentsInChildren<MeshFilter>();
                foreach (var mf in mfs)
                {
                    // TODO: unique mesh
                    meshes.Add(mf.sharedMesh);
                }
            }

            model.meshes = new Mesh[meshes.Count];
            for (int i = 0; i < model.meshes.Length; ++i)
            {
                model.meshes[i] = new Mesh();
                var mesh = model.meshes[i];
                mesh.name = meshes[i].name;
                mesh.primitives = new Primitive[meshes[i].subMeshCount];
                mesh.primitives[0] = new Primitive();
                mesh.primitives[0].attributes["POSITION"] = 0;
                mesh.primitives[0].attributes["NORMAL"] = 1;
                mesh.primitives[0].attributes["TEXCOORD_0"] = 2;
            }

            for (int i = 0; i < gos.Length; ++i)
            {
                var go = gos[i];
                model.CreateNode(go.transform);
            }

            string s = JsonUtility.ToJson(model);
            print(s);
        }
    }
}