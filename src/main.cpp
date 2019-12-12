#include <algorithm>
#include <iostream>
#include <sstream>

#include <dlib/cmd_line_parser.h>
#include <dlib/image_io.h>
#include <dlib/gui_widgets.h>
#include <dlib/dir_nav.h>
#include <dlib/image_transforms.h>

int main(int argc, char** argv) try
{
    dlib::command_line_parser cmd_parser;
    cmd_parser.add_option("max-size", "max num of pixels per image", 1);
    cmd_parser.add_option("max-images", "maximum number of images", 1);
    cmd_parser.set_group_name("Help Options");
    cmd_parser.add_option("h", "");
    cmd_parser.add_option("help", "display this message and exit");
    cmd_parser.parse(argc, argv);

    if (cmd_parser.number_of_arguments() == 0 || cmd_parser.option("h") || cmd_parser.option("help"))
    {
        std::cout << "\nUsage: tiler [options] directory\n\n";
        cmd_parser.print_options();
        return EXIT_SUCCESS;
    }

    const std::string& root = cmd_parser[0];
    const double max_size = dlib::get_option(cmd_parser, "max-size", std::numeric_limits<double>::max());
    const size_t max_images = dlib::get_option(cmd_parser, "max-images", std::numeric_limits<size_t>::max());
    auto files = dlib::get_files_in_directory_tree(root, dlib::match_endings(".jpg .JPG .jpeg .JPEG .png .PNG"));
    std::cout << "Found " << files.size() << " images\n";
    auto num_images = std::min(max_images, files.size());
    files.erase(files.begin() + num_images, files.end());
    std::cout << "Generating tiled image with " << files.size() << " images\n";

    std::vector<dlib::matrix<dlib::rgb_pixel>> images;
    images.assign(files.size(), dlib::matrix<dlib::rgb_pixel>());
    dlib::parallel_for(0, files.size(), [&files, max_size, &images](size_t i)
    {
        const auto& file = files[i];
        dlib::matrix<dlib::rgb_pixel> image;
        dlib::load_image(image, file.full_name());
        if (const auto scaling_factor = std::sqrt(max_size / image.size()); scaling_factor < 1)
        {
            dlib::resize_image(scaling_factor, image);
        }
        images[i] = std::move(image);
    });

    dlib::matrix<dlib::rgb_pixel> tiled_image = dlib::tile_images(images);
    dlib::save_jpeg(tiled_image, "tiled_image.jpg");
    dlib::image_window win(tiled_image, "tiled image");
    win.wait_until_closed();
    return EXIT_SUCCESS;
}
catch (std::exception& e)
{
    std::cout << e.what() << std::endl;
    return EXIT_FAILURE;
}
