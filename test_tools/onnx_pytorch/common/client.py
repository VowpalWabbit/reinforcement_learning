import tempfile
#from azure.storage.blob import BlockBlobService
import datetime
import os
import collections.abc
'''
class _AzureBlobIterator:
    def __init__(self, block_blob_service, container, path):
        local_copy = os.path.join(tempfile.mkdtemp(), "file.json")
        block_blob_service.get_blob_to_path(container, path, local_copy, max_connections=4)
        self.it = open(local_copy, 'r', encoding='utf-8')

    def __iter__(self):
        return self

    def __next__(self):
        return self.it.__next__()

class TenantStorageIterator:
    def __init__(self, context, start, end):
        self.context = context
        self.bbs = BlockBlobService(
            account_name=context.account_name,
            account_key=context.account_key
        )
        self.paths = list(
            filter(lambda path : self.bbs.exists(self.context.container, path), 
            map(lambda d : self.__date_2_path__(start + datetime.timedelta(d)), \
            range((end - start).days + 1))))

    def __iter__(self):
        for p in self.paths:
            az_it = _AzureBlobIterator(self.bbs, self.context.container, p)
            for l in az_it:
                yield l

    def __date_2_path__(self, date):
        return '%s/data/%s/%s/%s_0.json' % (
            self.context.folder,
            str(date.year),
            str(date.month).zfill(2),
            str(date.day).zfill(2)
        )

class _IndexedAzureBlobFile:
    @staticmethod
    def get(block_blob_service, container, path):
        from common import indexed_file
        import pandas as pd
        local_copy = os.path.join(tempfile.mkdtemp(), "file.json")
        block_blob_service.get_blob_to_path(container, path, local_copy, max_connections=4)
        return pd.DataFrame(data={'lines': open(local_copy, 'r').readlines()})


class TenantStorageDataset:
    @staticmethod
    def get(context, start, end):
        import pandas as pd
        bbs = BlockBlobService(
            account_name=context.account_name,
            account_key=context.account_key
        )
        paths = list(
            filter(lambda path : bbs.exists(context.container, path), 
            map(lambda d : TenantStorageDataset.__date_2_path__(context.folder, start + datetime.timedelta(d)), \
            range((end - start).days + 1))))
        return pd.concat(map(lambda p: _IndexedAzureBlobFile.get(bbs, context.container, p), paths))

    @staticmethod
    def __date_2_path__(folder, date):
        return '%s/data/%s/%s/%s_0.json' % (
            folder,
            str(date.year),
            str(date.month).zfill(2),
            str(date.day).zfill(2)
        )
'''
class LocalTextFileDataset:
    @staticmethod
    def get(filepath):
        import pandas as pd
        return pd.DataFrame(data={'lines': open(filepath, 'r').readlines()})

class LocalBinaryFileDataset:
    @staticmethod
    def get(filepath):
        import pandas as pd
        return pd.DataFrame(data={'lines': bytearray(open(filepath, 'rb').read())})
