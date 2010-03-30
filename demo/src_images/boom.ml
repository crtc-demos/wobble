let chunk_size = 441

let process_values buf obuf =
  let parts = (String.length buf) / chunk_size in
  for i = 0 to parts - 1 do
    let acc = ref 0 in
    for j = 0 to chunk_size - 1 do
      let idx = (i * chunk_size) + j in
      let around_zero = (Char.code buf.[idx]) - 128 in
      acc := !acc + abs around_zero
    done;
    obuf.(i) <- !acc / chunk_size
  done;
  let ostring = String.make parts '\000' in
  for i = 1 to parts - 2 do
    let smoothed = (obuf.(i - 1) / 2 + obuf.(i) + obuf.(i + 1) / 2) in
    ostring.[i] <- Char.chr (smoothed)
  done;
  ostring

let _ =
  let inp, outp = begin try
    Sys.argv.(1), Sys.argv.(2)
  with _ ->
    Printf.fprintf stderr "Usage: %s <in> <out>\n" Sys.argv.(0);
    exit 1
  end in
  let chin = open_in_bin inp in
  let inlength = in_channel_length chin in
  let buf = String.make inlength '\000' in
  let obuf = Array.make ((inlength + chunk_size - 1) / chunk_size) 0 in
  really_input chin buf 0 inlength;
  close_in chin;
  let ostring = process_values buf obuf in
  let chout = open_out_bin outp in
  output_string chout ostring;
  close_out chout
