
using Google.FlatBuffers;
using System.Collections.Generic;
using vwfb = VW.parsers.flatbuffer;

namespace Rl.Net.Flatbuffers
{
    public interface ICBExampleBuilder
    {
        bool BuildingShared { get; }
        int BuildingAction { get; }

        void PushNamespace(string name = "");

        void AddFeature(string name, float value = 1.0f);
        void AddFeature(string name, string value);

        int PushAction();

        byte[] FinishExample();
    }

    public class VW_CBExampleBuilder : ICBExampleBuilder
    {
        struct namespace_prototype
        {
            public string name;

            public List<StringOffset> feature_names;
            public List<float> feature_values;

            public Offset<vwfb.Namespace> Build(FlatBufferBuilder builder)
            {
                var nameOffset = builder.CreateString(this.name);

                var feature_names_array = vwfb.Namespace.CreateFeatureNamesVector(builder, this.feature_names.ToArray());
                var feature_indicies_array = vwfb.Namespace.CreateFeatureHashesVector(builder, new ulong[0]);
                var feature_values_array = vwfb.Namespace.CreateFeatureValuesVector(builder, this.feature_values.ToArray());

                // The presence of the name will get VW to compute a hash for us
                return vwfb.Namespace.CreateNamespace(builder, nameOffset, (byte)this.name[0], 0, feature_names_array, feature_values_array, feature_indicies_array);
            }
        }

        private List<Offset<vwfb.Example>> built_example_offsets;
        private List<namespace_prototype> namespaces = new List<namespace_prototype>();

        private FlatBufferBuilder builder;

        public VW_CBExampleBuilder()
        {
            this.builder = new FlatBufferBuilder(64); // todo - figure out right min size?
        }

        public void PushNamespace(string name = "")
        {
            this.namespaces.Add(new namespace_prototype()
            {
                name = name,
                feature_names = new List<StringOffset>(),
                feature_values = new List<float>()
            });
        }

        public void AddFeature(string name, string value)
        {
            this.AddFeature(name + "_" + value);
        }

        public void AddFeature(string name, float value = 1.0f)
        {
            if (this.namespaces.Count == 0)
            {
                // TODO: is this the right hash?
                this.PushNamespace("");
            }

            StringOffset nameOffset = this.builder.CreateString(name);
            this.namespaces[this.namespaces.Count - 1].feature_names.Add(nameOffset);
            this.namespaces[this.namespaces.Count - 1].feature_values.Add(value);
        }

        public int PushAction()
        {
            this.CollectExample();

            return this.built_example_offsets.Count - 1;
        }

        private void CollectExample()
        {
            Offset<vwfb.Namespace>[] namespaceOffsets = new Offset<vwfb.Namespace>[this.namespaces.Count];
            for (int i = 0; i < this.namespaces.Count; i++)
            {
                namespaceOffsets[i] = this.namespaces[i].Build(this.builder);
            }

            var namespacesVector = vwfb.Example.CreateNamespacesVector(this.builder, namespaceOffsets);
            var result = vwfb.Example.CreateExample(this.builder, namespacesVector);
        }

        public byte[] FinishExample()
        {
            this.CollectExample();

            var multi_ex = vwfb.MultiExample.CreateMultiExample(this.builder, vwfb.MultiExample.CreateExamplesVector(this.builder, this.built_example_offsets.ToArray()));
            var root = vwfb.ExampleRoot.CreateExampleRoot(this.builder, vwfb.ExampleType.MultiExample, multi_ex.Value);

            this.builder.FinishSizePrefixed(root.Value);

            return this.builder.SizedByteArray();
        }

        public bool BuildingShared { get => this.built_example_offsets.Count == 0; }
        public int BuildingAction { get => this.built_example_offsets.Count - 1; }
    }
}