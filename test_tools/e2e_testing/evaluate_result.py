#! /usr/bin/env python3 -W ignore::DeprecationWarning

from vw_executor.artifacts import Output

import argparse
import numpy as np


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--base_file", type=str, help="Base file location")
    parser.add_argument("--compare_file", help="Comparison file location")
    args = parser.parse_args()

    vw_base = Output(args.base_file)
    vw_compare = Output(args.compare_file)

    if not np.isclose([vw_base.loss], [vw_compare.loss]):
        print("base.loss_table:")
        print(vw_base.loss_table)
        print("base.metrics:")
        print(vw_base.metrics)
        print("compare.loss_table:")
        print(vw_compare.loss_table)
        print("compare.metrics:")
        print(vw_compare.metrics)
        raise Exception(
            f"Loss is different between base and compare: {vw_base.loss} vs. {vw_compare.loss}"
        )

    if vw_base.loss_table.shape != vw_compare.loss_table.shape:
        raise Exception("Base and compare loss tables are a different shape")

    for i in range(len(vw_base.loss_table)):
        if not np.isclose(
            vw_base.loss_table.iloc[i], vw_compare.loss_table.iloc[i]
        ).all():
            raise Exception("Loss is different between base and compare")

    print("Outputs match")


if __name__ == "__main__":
    main()
