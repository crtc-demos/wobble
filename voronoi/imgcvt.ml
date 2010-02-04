type point = {
  x : int;
  y : int;
  c : int
}

let xsize = 640
and ysize = 480

let random_points () =
  let pts = Array.create 64 { x = 0; y = 0; c = 0 } in
  for i = 0 to 63 do
    pts.(i) <- {
      x = Random.int xsize;
      y = Random.int ysize;
      c = Graphics.rgb (Random.int 256) (Random.int 256) (Random.int 256)
    }
  done;
  pts

let closest pts x y =
  let found = ref None
  and closest = ref max_int in
  for i = 0 to 63 do
    let pt = pts.(i) in
    let sqdist = (pt.y - y) * (pt.y - y)
                 + (pt.x - x) * (pt.x - x) in
    if sqdist < !closest then begin
      found := Some i;
      closest := sqdist
    end
  done;
  match !found with
    None -> raise Not_found
  | Some x -> x

let slow_voronoi pts =
  Graphics.display_mode false;
  for y = 0 to ysize - 1 do
    for x = 0 to xsize - 1 do
      let clo = closest pts x y in
      Graphics.set_color pts.(clo).c;
      Graphics.plot x y
    done;
    if (y land 3) = 3 then
      Graphics.synchronize ()
  done

let _ =
  let pts = random_points () in
  let img = Images.load "testimg.png" [] in
  Graphics.open_graph "";
  Graphics.set_window_title "It's a graphics window";
  Graphics.resize_window xsize ysize;
  Graphic_image.draw_image img 0 0;
  slow_voronoi pts;
  while true do
    let evs = Graphics.wait_next_event [Graphics.Button_down] in
    match evs with
      { Graphics.button = bt } when bt = true -> exit 0
    | _ -> ()
  done
