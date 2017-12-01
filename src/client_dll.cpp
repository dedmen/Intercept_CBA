#include <intercept.hpp>
#include "SQFExtension/SQFExtensions.hpp"

int intercept::api_version() {
    return 1;
}

void  intercept::on_frame() {

}

void intercept::pre_start() {
    SQFExtensions::Utility::preStart();
    SQFExtensions::Math::preStart();
}

void  intercept::pre_init() {

}

void intercept::post_init() {

}

void intercept::mission_ended() {

}
