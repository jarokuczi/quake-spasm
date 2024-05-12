cmake_minimum_required(VERSION 3.16.3)

project(quake)

set(q_source
        bgmusic.c snd_codec.c snd_flac.c snd_wave.c snd_vorbis.c snd_opus.c snd_mp3tag.c
        snd_mikmod.c snd_modplug.c snd_xmp.c snd_umx.c snd_dma.c snd_mix.c snd_mem.c snd_sdl.c
        cd_sdl.c in_sdl.c gl_vidsdl.c net_bsd.c net_udp.c pl_linux.c sys_sdl_unix.c main_sdl.c
        gl_refrag.c gl_rlight.c gl_rmain.c gl_fog.c gl_rmisc.c r_part.c r_world.c gl_screen.c
        gl_sky.c gl_warp.c gl_draw.c image.c gl_texmgr.c gl_mesh.c r_sprite.c r_alias.c r_brush.c
        gl_model.c strlcat.c strlcpy.c net_dgrm.c net_loop.c net_main.c chase.c cl_demo.c cl_input.c
        cl_main.c cl_parse.c cl_tent.c console.c keys.c menu.c sbar.c view.c wad.c cmd.c common.c
        miniz.c crc.c cvar.c cfgfile.c host.c host_cmd.c mathlib.c pr_cmds.c pr_edict.c pr_exec.c
        sv_main.c sv_move.c sv_phys.c sv_user.c world.c zone.c
)

add_executable(quake ${q_source})

target_include_directories(quake PRIVATE . /usr/local/amiga/ppc-amigaos/SDK/include/netinclude)

target_link_libraries(quake PUBLIC m gl4es SDL2)

target_link_options(quake PRIVATE  -athread=native)

target_compile_options(quake PUBLIC -D__amigaos4__ -DPLATFORM_AMIGA -DUSE_SDL2 -DAMIGA "-D_inline=static __inline" "-D__int64=int64"  -D__stdcall= -DENDIAN_INLINE -DNDEBUG -O3 -fPIC -DSOUND_IMPLEMENTED -DOGG)

set (CMAKE_SHARED_LIBRARY_PREFIX_C "")
