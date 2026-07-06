#include "cli.hpp"

int main(int argc, char **argv) {
    cli::init(argc, argv);
    cli::parse_collect();
    cli::execute();
}
