from torch.utils.data.dataset import IterableDataset, Dataset
from torch import tensor, randn, onnx, is_tensor

from common import types

class IterableLogs(IterableDataset):
    def __init__(self, iterator, transform = None):
        self.it = iterator
        self.transform = transform

    def __iter__(self):
        for l in self.it:
            ret = l
            if self.transform:
                ret = self.transform(l)
            if ret:
                yield ret

# input is a dataframe containing a line of dsjson
class Logs(Dataset):
    def __init__(self, df, transform = None):
        self.df = df['lines']
        self.df = self.df[self.df.str.startswith('{"RewardValue') == False]
        if transform:
            transformed = self.df.apply(lambda l: transform(l))
            self.df = transformed[transformed.isnull() == False]
        self.df = self.df.values

    def __len__(self):
        return len(self.df)

    def __getitem__(self, idx):
        return self.df[idx]

# input is a dict {'features': dict { key: features }, 'label': label_index, 'cost': cost }
class DictToCbTensor(object):
    def __init__(self, problem_type = types.Problem.MultiClass):
        self.problem_type = problem_type

    def __call__(self, example):
        from common.parser import CbDictParser

        parsed = CbDictParser.parse(example)
        #only for 1 tensor so far
        features = {}
        for k, v in parsed['features'].items():
            features = tensor(v)
            break

        if self.problem_type == types.Problem.MultiClass and parsed['cost'] == 0:
            return None

        return features, parsed['label'], 

class Model:
    @staticmethod
    def export(model, device, path):
        dummy_input = randn(1, 1, 28, 28, device=device)
        onnx.export(model, dummy_input, path, \
                input_names = ['Input3'], \
                output_names = ['Plus214_Output_0'])
