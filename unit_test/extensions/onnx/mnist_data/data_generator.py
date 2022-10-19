#%%

from mnist.loader import MNIST;
import base64;
import struct;


#%%
mndata = MNIST('./data')
images, labels = mndata.load_testing()

num_images = len(images);
print(num_images);

#%%

def map_pixel_float(p):
    return float(p) / 256.0;


#%%

def image_to_float(image):
    return map(map_pixel_float, image);


#%%

def image_to_bytes(mnist_image):
    pixel_count = len(mnist_image);
    image_float = map(lambda pixel_int: float(pixel_int) / 256.0, mnist_image);
    
    return struct.pack('%sf' % pixel_count, *image_float);


#%%

dims = [1, 1, 28, 28]
prefix = "\"" + base64.b64encode(struct.pack('4Q', *dims)).decode() + ";"

def image_to_tensor_notation(mnist_image):
    return prefix + base64.b64encode(image_to_bytes(mnist_image)).decode() + "\"";


#%%

def encode_mnist(file, images, labels):
    num_images = len(images);

    for i in range(num_images):
        file.write(str(labels[i]));
        file.write('|');
        file.write(image_to_tensor_notation(images[i]));
        file.write('\n');


#%%

tensor_notation_file = open('mnist_test_data.txt', 'wt');

encode_mnist(tensor_notation_file, images, labels);

tensor_notation_file.flush();
tensor_notation_file.close();