set(req_components esp_hap_core)
list(APPEND req_components esp_hap_apple_profiles)
list(APPEND req_components button)
list(APPEND req_components app_wifi)
list(APPEND req_components app_hap_setup_payload)

idf_component_register(SRCS "hapc.c" INCLUDE_DIRS "." REQUIRES "${req_components}")
