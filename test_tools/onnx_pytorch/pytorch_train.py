import argparse
import os
import datetime
from shutil import copyfile
from adapters import pytorch
import argparse
import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
from torchvision import datasets, transforms
from torch.optim.lr_scheduler import StepLR
import numpy as np
from torch.utils.data.sampler import SubsetRandomSampler


class Net(nn.Module):
    def __init__(self):
        super(Net, self).__init__()
        self.conv1 = nn.Conv2d(1, 10, kernel_size=5)
        self.conv2 = nn.Conv2d(10, 20, kernel_size=5)
        self.conv2_drop = nn.Dropout2d()
        self.fc1 = nn.Linear(320, 50)
        self.fc2 = nn.Linear(50, 10)

    def forward(self, x):
        x = F.relu(F.max_pool2d(self.conv1(x), 2))
        x = F.relu(F.max_pool2d(self.conv2_drop(self.conv2(x)), 2))
        x = x.view(-1, 320)
        x = F.relu(self.fc1(x))
        x = F.dropout(x, training=self.training)
        x = self.fc2(x)
        return F.log_softmax(x, dim=1)


def train(model, device, train_loader, optimizer, epoch, output_dir):
    model.train()
    for batch_idx, (data, target) in enumerate(train_loader):
        data, target = data.to(device), target.to(device)
        optimizer.zero_grad()
        output = model(data.float())
        loss = F.nll_loss(output, target)
        loss.backward()
        optimizer.step()
        print("Train: [{}]\tLoss: {:.6f}".format(batch_idx * len(data), loss.item()))


def test(model, device, test_loader):
    model.eval()
    test_loss = 0
    correct = 0
    with torch.no_grad():
        for data, target in test_loader:
            data, target = data.to(device), target.to(device)
            output = model(data.float())
            test_loss += F.nll_loss(
                output, target, size_average=False, reduce=True
            ).item()  # sum up batch loss
            pred = output.max(1, keepdim=True)[
                1
            ]  # get the index of the max log-probability
            correct += pred.eq(target.view_as(pred)).sum().item()

    test_loss /= len(test_loader.dataset)
    print(
        "\nTest set: Average loss: {:.4f}, Accuracy: {}/{} ({:.0f}%)\n".format(
            test_loss,
            correct,
            len(test_loader.dataset),
            100.0 * correct / len(test_loader.dataset),
        )
    )


def split_train_validation(ds, batch_size, ratio):
    ds_size = len(ds)
    indices = list(range(ds_size))
    split = int(np.floor(ratio * ds_size))

    train_indices, val_indices = indices[split:], indices[:split]

    # Creating PT data samplers and loaders:
    train_sampler = SubsetRandomSampler(train_indices)
    valid_sampler = SubsetRandomSampler(val_indices)

    train_loader = torch.utils.data.DataLoader(
        ds, batch_size=batch_size, sampler=train_sampler
    )
    test_loader = torch.utils.data.DataLoader(
        ds, batch_size=batch_size, sampler=valid_sampler
    )
    return train_loader, test_loader


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--batch-size",
        type=int,
        default=64,
        metavar="N",
        help="input batch size for training (default: 64)",
    )
    parser.add_argument("--output-dir", type=str, default="outputs")
    parser.add_argument("--start-date", type=str)
    parser.add_argument("--end_date", type=str)
    parser.add_argument("filename", type=str)
    args = parser.parse_args()

    output_dir = args.output_dir
    os.makedirs(output_dir, exist_ok=True)

    data = bytearray(open(args.filename, "rb").read())
    from common.parser import VWFlatbufferParser

    ds = pytorch.IterableLogs(VWFlatbufferParser(data), pytorch.DictToCbTensor())

    train_loader = torch.utils.data.DataLoader(
        ds, batch_size=args.batch_size, num_workers=0
    )

    torch.manual_seed(1)
    device = torch.device("cpu")

    model = Net().to(device)
    optimizer = optim.SGD(model.parameters(), lr=0.01, momentum=0.5)

    for epoch in range(1, 6):
        train(model, device, train_loader, optimizer, epoch, output_dir)

    model_path = os.path.join(output_dir, "current.onnx")
    pytorch.Model.export(model, device, model_path)


if __name__ == "__main__":
    main()
