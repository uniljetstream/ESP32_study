set(srcs
        
         "queue_example.c"
        
        
        )
idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES console esp_timer)
