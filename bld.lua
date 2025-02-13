


return {


   {


      not_dependencies = {



         "lfs",



         "resvg",

      },



      artifact = "pomidor",

      main = "main.c",

      src = "src",

      exclude = {},



      debug_define = {
         ["BOX2C_SENSOR_SLEEP"] = "1",
      },

      release_define = {},



   },


}
