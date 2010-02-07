open Genlex

type point = {
  x : int;
  y : int;
  mutable c : int
}

let xsize = 640
and ysize = 480
and num_points = 64 (* 64 *)
and population_size = 32 (* 32 *)
and breeding_pairs = 16 (* 16 *)
and mutation_rate = 2

let best_score = ref max_int
let best_points = ref None

let mk_xpoint () =
  Random.int xsize

let mk_ypoint () =
  Random.int ysize

let random_points () =
  let pts = Array.create num_points { x = 0; y = 0; c = 0 } in
  for i = 0 to num_points - 1 do
    pts.(i) <- {
      x = mk_xpoint ();
      y = mk_ypoint ();
      c = Graphics.rgb (Random.int 256) (Random.int 256) (Random.int 256)
    }
  done;
  pts

let closest pts x y =
  let found = ref None
  and closest = ref max_int in
  for i = 0 to num_points - 1 do
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

exception Found of int

let slow_scan pts num scanline ~left =
  try
    begin
      if left then begin
	for x = 0 to xsize - 1 do
	  if closest pts x scanline = num then
            raise (Found x)
	done
      end else begin
	for x = xsize - 1 downto 0 do
          if closest pts x scanline = num then
	    raise (Found x)
	done
      end
    end;
    None
  with Found x -> Some x

let find_endpoint pts num scanline ~left =
  let l, r =
    if left then
      0, pts.(num).x
    else
      pts.(num).x, xsize - 1 in
  let rec scan lhs lhspt rhs rhspt =
    let mid = (lhs + rhs) / 2 in
    let midpt = closest pts mid scanline in
    if abs (lhs - rhs) < 2 then
      Some mid
    else begin
      match lhspt = num, midpt = num, rhspt = num with
        true, true, false ->
	  if left then
	    Some lhs
	  else
	    scan mid midpt rhs rhspt
      | true, false, false ->
          if left then
	    Some lhs
	  else
	    scan lhs lhspt mid midpt
      | false, true, true ->
          if not left then
	    Some rhs
	  else
	    scan lhs lhspt mid midpt
      | false, false, true ->
          if not left then
	    Some rhs
	  else
	    scan mid midpt rhs rhspt
      | true, true, true -> if left then Some lhs else Some rhs
      | _ -> slow_scan pts num scanline ~left
    end in
  scan l (closest pts l scanline) r (closest pts r scanline)

let find_endpoints pts num scanline =
  let lhs = find_endpoint pts num scanline ~left:true
  and rhs = find_endpoint pts num scanline ~left:false in
  match lhs, rhs with
    Some lhs, Some rhs -> lhs, rhs
  | _ -> raise Not_found

let get_voronoi_array_slow pts pt =
  let outp = ref [] in
  try
    for y = pts.(pt).y to ysize - 1 do
      let l, r = find_endpoints pts pt y in
      outp := (l, r, y) :: !outp
    done;
    raise Not_found
  with Not_found ->
    begin try
      for y = pts.(pt).y - 1 downto 0 do
        let l, r = find_endpoints pts pt y in
	outp := (l, r, y) :: !outp
      done;
      raise Not_found
    with Not_found ->
      !outp
    end

let get_voronoi_array pts =
  let diag = Array.create num_points [] in
  for pt = 0 to num_points - 1 do
    diag.(pt) <- get_voronoi_array_slow pts pt
  done;
  diag

let poll_until_click () =
  while true do
    let evs = Graphics.wait_next_event [Graphics.Button_down] in
    match evs with
      { Graphics.button = bt } when bt = true -> exit 0
    | _ -> ()
  done

let lexer = Genlex.make_lexer ["v"; "e"; "l"; "s"]

type recs = Vertex of float * float
          | Edge of int * int * int
	  | Line of float * float * float
	  | Source of float * float

let rec parse_line = parser
    [< 'Kwd "v"; 'Float vx; 'Float vy >] -> Vertex (vx, vy)
  | [< 'Kwd "e"; 'Int ln; 'Int pt1; 'Int pt2 >] -> Edge (ln, pt1, pt2)
  | [< 'Kwd "l"; 'Float a; 'Float b; 'Float c >] -> Line (a, b, c)
  | [< 'Kwd "s"; 'Float a; 'Float b >] -> Source (a, b)
and parse_file = parser
    [< line = parse_line; rest = parse_file >] ->
      let vts, edges, lines = rest in
      begin match line with
        Vertex (vx, vy) -> (vx, vy)::vts, edges, lines
      | Edge (a, b, c) -> vts, (a, b, c)::edges, lines
      | Line (a, b, c) -> vts, edges, (a, b, c)::lines
      | Source _ -> vts, edges, lines
      end
  | [< >] -> [], [], []

let sqdist x1 y1 x2 y2 =
  let xd = x2 -. x1
  and yd = y2 -. y1 in
  (xd *. xd) +. (yd *. yd)

let point_on_edge x1 y1 a b c =
  (* ax + by = c
     y = (c - ax) / b  or
     x = (c - by) / a
  *)
  let edge_intersections = [
    (* lhs *)
    0.0, c /. b;
    (* rhs *)
    float_of_int xsize, (c -. a *. (float_of_int xsize)) /. b;
    (* bot *)
    c /. a, 0.0;
    (* top *)
    (c -. b *. (float_of_int ysize)) /. a, float_of_int ysize] in
  let with_dist =
    List.map (fun (x, y) -> x, y, sqdist x1 y1 x y) edge_intersections in
  let sorted =
    List.sort (fun (_, _, d1) (_, _, d2) -> compare d1 d2) with_dist in
  let x, y, _ = List.hd sorted in
  x, y

let right_segs arr =
  for i = 0 to Array.length arr - 1 do
    let (pt1, x1, y1, pt2, x2, y2) = arr.(i) in
    if y2 < y1 then
      arr.(i) <- pt2, x2, y2, pt1, x1, y1
  done

let segs_around_pt segs ptx pty =
  let ptx = float_of_int ptx and pty = float_of_int pty in
  let intersecting = ref [] in
  Array.iteri
    (fun n (_, x1, y1, _, x2, y2) ->
      if (pty >= y1 && pty <= y2) || (pty >= y2 && pty <= y1) then
        intersecting := n :: !intersecting)
    segs;
  let intersections = List.map
    (fun n ->
      let (_, x1, y1, _, x2, y2) = segs.(n) in
      let param = (pty -. y1) /. (y2 -. y1) in
      n, x1 +. param *. (x2 -. x1))
    !intersecting in
  let sorted_intersections = List.sort
    (fun (_, x1) (_, x2) -> compare x1 x2)
    intersections in
  let rec scan = function
    (n1, x1) :: (n2, x2) :: _ when ptx >= x1 && ptx <= x2 -> n1, n2
  | [n1, x1] when ptx >= x1 -> n1, -1
  | (n1, x1) :: rest when ptx <= x1 -> -1, n1
  | pt :: rest -> scan rest
  | [] -> -1, -1 in
  scan sorted_intersections

let rec segs_connected_to_vertex segs vtx grad ~top ~left =
  let connected = ref [] in
  Array.iteri
    (fun n (pt1, x1, y1, pt2, x2, y2) ->
      if (top && vtx = pt1) || ((not top) && vtx = pt2) then begin
	let gradient = (x2 -. x1) /. (y2 -. y1) in
	connected := (n, gradient) :: !connected
      end)
    segs;
  let sorted = List.sort
    (fun (_, g1) (_, g2) ->
      if left then
        compare g1 g2
      else
        compare g2 g1)
    !connected in
  match sorted with
    (con, newgrad) :: _ ->
      let (pt1, _, _, pt2, _, _) = segs.(con) in
      if top then begin
        if ((left && grad > newgrad) || ((not left) && grad < newgrad))
	   && pt2 <> -1 then
          con :: segs_connected_to_vertex segs pt2 newgrad ~top ~left
	else
	  []
      end else begin
        if ((left && grad > newgrad) || ((not left) && grad < newgrad))
	   && pt1 <> -1 then
          con :: segs_connected_to_vertex segs pt1 newgrad ~top ~left
	else
	  []
      end
  | _ -> []

type active_seg = {
  mutable xpos : float;
  mutable xdelta : float;
  mutable yleft : int
}

let xclip v =
  if v <= 0 then 0 else if v >= xsize then xsize - 1 else v

let yclip segs =
  List.map
    (fun (x1, y1, x2, y2) ->
      if y1 < 0 then
        let param = (float_of_int (- y1)) /. (float_of_int (y2 - y1)) in
	let xmid = x1 +. param *. (x2 -. x1) in
	(xmid, 0, x2, y2)
      else
        (x1, y1, x2, y2))
    segs

let clip_left x1 y1 x2 y2 =
  if x1 < 0.0 then
    let param = -. x1 /. (x2 -. x1) in
    (0.0, y1 +. param *. (y2 -. y1))
  else 
    (x1, y1)

let clip_right x1 y1 x2 y2 =
  let xsize = float_of_int xsize in
  if x1 >= xsize then
    let param = (xsize -. x1) /. (x2 -. x1) in
    (xsize, y1 +. param *. (y2 -. y1))
  else
    (x1, y1)

let clip_top x1 y1 x2 y2 =
  if y1 < 0.0 then
    let param = -. y1 /. (y2 -. y1) in
    (x1 +. param *. (x2 -. x1), 0.0)
  else
    (x1, y1)

let clip_bottom x1 y1 x2 y2 =
  let ysize = float_of_int ysize in
  if y1 >= ysize then
    let param = (ysize -. y1) /. (y2 -. y1) in
    (x1 +. param *. (x2 -. x1), ysize)
  else
    (x1, y1)

let edge_clip x1 y1 x2 y2 =
  let x1, y1 = clip_left x1 y1 x2 y2 in
  let x2, y2 = clip_left x2 y2 x1 y1 in
  let x1, y1 = clip_right x1 y1 x2 y2 in
  let x2, y2 = clip_right x2 y2 x1 y1 in
  let x1, y1 = clip_top x1 y1 x2 y2 in
  let x2, y2 = clip_top x2 y2 x1 y1 in
  let x1, y1 = clip_bottom x1 y1 x2 y2 in
  let x2, y2 = clip_bottom x2 y2 x1 y1 in
  x1, y1, x2, y2    

exception Too_difficult

let rec rasterize ~have_left ~have_right raster last active segs spans =
  let new_active, new_segs =
    List.fold_right
      (fun seg (actv, newsegs) ->
	let (x1, y1, x2, y2) = seg in
	if raster >= y1 then
	  let newact = {
	    xpos = x1;
	    xdelta = (x2 -. x1) /. (float_of_int (y2 - y1));
	    yleft = y2 - y1
	  } in
	  (newact :: actv, newsegs)
	else
	  actv, seg :: newsegs)
      segs
      (active, []) in
  let new_active =
    List.fold_right
      (fun act acts ->
	act.xpos <- act.xpos +. act.xdelta;
	act.yleft <- act.yleft - 1;
	if act.yleft < 0 then
	  acts
	else
	  act :: acts)
      new_active
      [] in
  (*Printf.printf "active: %d, raster: %d, have_left: %s, have_right: %s\n"
    (List.length new_active) raster (if have_left then "true" else "false")
    (if have_right then "true" else "false");
  flush stdout;*)
  let new_spans = match new_active with
    [a; b] ->
      let side1 = int_of_float a.xpos
      and side2 = int_of_float b.xpos in
      if side2 < side1 then
	(xclip side2, xclip side1, raster) :: spans
      else
	(xclip side1, xclip side2, raster) :: spans
  | [a] -> raise Too_difficult
  | _ -> spans in
  if raster < last - 1 then
    rasterize ~have_left ~have_right (succ raster) last
	      new_active new_segs new_spans
  else
    new_spans

let get_voronoi_array_ext pts =
  let pin, pout = Unix.open_process "fortune/voronoi" in
  for pt = 0 to num_points - 1 do
    Printf.fprintf pout "%d %d\n" pts.(pt).x pts.(pt).y
  done;
  flush pout;
  close_out pout;
  let vts, edges, lines = parse_file (lexer (Stream.of_channel pin)) in
  ignore (Unix.close_process (pin, pout));
  let vtarr = Array.of_list vts
  and edgarr = Array.of_list edges
  and linarr = Array.of_list lines in
  let segs = ref [] in
  for pt = 0 to Array.length edgarr - 1 do
    let lseg, pt1, pt2 = edgarr.(pt) in
    if pt1 <> -1 && pt2 <> -1 then begin
      let x1, y1 = vtarr.(pt1) and x2, y2 = vtarr.(pt2) in
      let x1, y1, x2, y2 = edge_clip x1 y1 x2 y2 in
      segs := (pt1, x1, y1, pt2, x2, y2) :: !segs
    end else if pt1 <> -1 then begin
      let a, b, c = linarr.(lseg) in
      let x1, y1 = vtarr.(pt1) in
      let x, y = point_on_edge x1 y1 a b c in
      segs := (pt1, x1, y1, pt2, x, y) :: !segs
    end else if pt2 <> -1 then begin
      let a, b, c = linarr.(lseg) in
      let x2, y2 = vtarr.(pt2) in
      let x, y = point_on_edge x2 y2 a b c in
      segs := (pt1, x, y, pt2, x2, y2) :: !segs
    end
  done;
  (* Graphics.set_color Graphics.red;
  List.iter
    (fun (_, x1, y1, _, x2, y2) ->
      Graphics.moveto (int_of_float x1) (int_of_float y1);
      Graphics.lineto (int_of_float x2) (int_of_float y2))
    !segs; *)
  let seg_arr = Array.of_list !segs in
  right_segs seg_arr;
  let diag = Array.create num_points [] in
  for pt = 0 to num_points - 1 do
    let segs_for_pt = ref [] in
    let n1, n2 = segs_around_pt seg_arr pts.(pt).x pts.(pt).y in
    if n1 <> -1 && n2 <> -1 then begin
      (* Left side.  *)
      let (pt1, x1, y1, pt2, x2, y2) = seg_arr.(n1) in
      segs_for_pt := n1 :: !segs_for_pt;
      let grad = (x2 -. x1) /. (y2 -. y1) in
      let cons = segs_connected_to_vertex seg_arr pt1 grad
					  ~top:false ~left:true in
      segs_for_pt := cons @ !segs_for_pt;
      let cons = segs_connected_to_vertex seg_arr pt2 grad
					  ~top:true ~left:false in
      segs_for_pt := cons @ !segs_for_pt;

      (* Right side.  *)
      let (pt1, x1, y1, pt2, x2, y2) = seg_arr.(n2) in
      segs_for_pt := n2 :: !segs_for_pt;
      let grad = (x2 -. x1) /. (y2 -. y1) in
      let cons = segs_connected_to_vertex seg_arr pt1 grad
					  ~top:false ~left:false in
      segs_for_pt := cons @ !segs_for_pt;
      let cons = segs_connected_to_vertex seg_arr pt2 grad
					  ~top:true ~left:true in
      segs_for_pt := cons @ !segs_for_pt;

      let segs = List.map
	(fun n ->
	  let (_, x1, y1, _, x2, y2) = seg_arr.(n) in
	  (x1, int_of_float y1, x2, int_of_float y2))
	!segs_for_pt in
      let sorted_segs = List.sort
	(fun (_, y1, _, _) (_, y2, _, _) -> compare y1 y2)
	segs in
      let clipped_sorted_segs = yclip sorted_segs in
      (*Graphics.set_color Graphics.green;
      List.iter
	(fun (x1, y1, x2, y2) ->
          Graphics.moveto (int_of_float x1) y1;
	  Graphics.lineto (int_of_float x2) y2)
	clipped_sorted_segs; *)
      (* poll_until_click (); *)
      try
	begin match clipped_sorted_segs with
	  hd::_ ->
	    let (_, top, _, _) = hd in
	    let (_, _, _, bot) = List.hd (List.rev clipped_sorted_segs) in
	    let bot = if bot > ysize then ysize else bot in
	    if top >= 0 then begin
	      let spans = rasterize ~have_left:(n1 <> -1) ~have_right:(n2 <> -1)
				    top bot [] clipped_sorted_segs [] in
	      diag.(pt) <- spans
	    end
	| _ -> ()
	end
      with Too_difficult ->
        diag.(pt) <- get_voronoi_array_slow pts pt
    end else
      diag.(pt) <- get_voronoi_array_slow pts pt
  done;
  diag

let rec get_voronoi_array_ext' pts =
  try
    get_voronoi_array_ext pts
  with ((Unix.Unix_error _) as e) ->
    raise e
  | Stream.Error _ ->
    (* This sometimes happens. Fall back to slow algorithm.  *)
    get_voronoi_array pts

let img_rgb img x y =
  match img with
    Images.Rgb24 (rgbimg) ->
      let col = Rgb24.get rgbimg x (ysize - 1 - y) in
      let { Color.Rgb.r = r; Color.Rgb.g = g; Color.Rgb.b = b } = col in
      r, g, b
  | Images.Rgba32 (rgbaimg) ->
      let { Color.Rgba.color = col } = Rgba32.get (rgbaimg) x (ysize - 1 - y) in
      let { Color.Rgb.r = r; Color.Rgb.g = g; Color.Rgb.b = b } = col in
      r, g, b
  | _ -> failwith "Unrecognized image format"

let get_region_colour img arr region =
  let racc = ref 0 and gacc = ref 0 and bacc = ref 0 and pixels = ref 0 in
  List.iter
    (fun (lf, rt, y) ->
      for x = lf to rt do
        let r, g, b = img_rgb img x y in
        racc := !racc + r;
	gacc := !gacc + g;
	bacc := !bacc + b;
	incr pixels
      done)
    arr.(region);
  if !pixels = 0 then
    0, 0, 0
  else
    !racc / !pixels, !gacc / !pixels, !bacc / !pixels

let score_region img arr region (r, g, b) =
  let diff = ref 0.0 and pixels = ref 0 in
  List.iter
    (fun (lf, rt, y) ->
      for x = lf to rt do
        let ir, ig, ib = img_rgb img x y in
	let dr, dg, db = ir - r, ig - g, ib - b in
	diff := !diff
	  +. (float_of_int ((dr * dr) + (dg * dg) + (db * db)) (* ** 1.5 *));
	incr pixels
      done)
    arr.(region);
  !diff

let colour_all_regions img pts arr =
  let score = ref 0.0 in
  for pt = 0 to num_points - 1 do
    let (r, g, b) as col = get_region_colour img arr pt in
    pts.(pt).c <- Graphics.rgb r g b;
    score := !score +. score_region img arr pt col
  done;
  let int_score = int_of_float (!score /. (float_of_int xsize
					   *. float_of_int ysize)) in
  if int_score < !best_score then begin
    best_score := int_score;
    best_points := Some (Array.copy pts)
  end;
  Printf.printf "score: %d (%d)\n" int_score !best_score;
  flush stdout;
  int_score

let render_voronoi_array pts arr =
  for pt = 0 to num_points - 1 do
    Graphics.set_color pts.(pt).c;
    List.iter
      (fun (l, r, y) ->
        Graphics.moveto l y;
	Graphics.lineto r y)
      arr.(pt)
  done;
  Graphics.set_color Graphics.black;
  for pt = 0 to num_points - 1 do
    Graphics.fill_circle pts.(pt).x pts.(pt).y 4
  done

let seed_world img num =
  let cloud = Array.create num ([| |], 0) in
  for item = 0 to num - 1 do
    let pts = random_points () in
    let varr = get_voronoi_array_ext' pts in
    let score = colour_all_regions img pts varr in
    cloud.(item) <- pts, score;
    render_voronoi_array pts varr;
    (*poll_until_click ()*)
  done;
  cloud

let shuffle pts =
  for i = 0 to num_points - 2 do
    let newpos = i + Random.int ((num_points - 1) - i) in
    if i <> newpos then begin
      let tmp = pts.(i) in
      pts.(i) <- pts.(newpos);
      pts.(newpos) <- tmp
    end
  done

let breed_points pts1 pts2 =
  let has_pt = Hashtbl.create 64 in
  let new_pts = Array.create num_points { x = 0; y = 0; c = 0 } in
  for i = 0 to num_points - 1 do
    if (Random.int 256) < mutation_rate then begin
      (* Mutation!!!  *)
      new_pts.(i) <- { x = mk_xpoint (); y = mk_ypoint (); c = 0 }
    end else begin
      let rec retry () =
	let nx, ny = if Random.bool () then
	  pts1.(i).x, pts1.(i).y
	else
          pts2.(i).x, pts2.(i).y in
	if Hashtbl.mem has_pt (nx, ny) then begin
	  (* A duplicate point? Try again!  *)
          shuffle pts1;
	  shuffle pts2;
	  retry ()
	end else begin
	  new_pts.(i) <- { x = nx; y = ny; c = 0 };
	  Hashtbl.add has_pt (nx, ny) true
	end in
      retry ()
    end
  done;
  (* shuffle new_pts;*)
  new_pts

(* Random.int twice skews numbers to low end.  *)
let parents () =
  let a = Random.int (1 + Random.int breeding_pairs) in
  let b = ref a in
  while a = !b do
    b := Random.int (1 + Random.int breeding_pairs)
  done;
  a, !b

let generate img cloud newsize =
  let new_cloud = Array.create newsize ([| |], 0) in
  Array.sort (fun (_, sc1) (_, sc2) -> compare sc1 sc2) cloud;
  for i = 0 to newsize - 1 do
    let par_a, par_b = parents () in
    let newpar1, _ = cloud.(par_a)
    and newpar2, _ = cloud.(par_b) in
    let newpts = breed_points newpar1 newpar2 in
    let varr = get_voronoi_array_ext' newpts in
    let score = colour_all_regions img newpts varr in
    new_cloud.(i) <- newpts, score;
    render_voronoi_array newpts varr
  done;
  new_cloud

let _ =
  Random.self_init ();
  let img = Images.load "testimg.png" [] in
  Graphics.open_graph "";
  Graphics.set_window_title "It's a graphics window";
  Graphics.resize_window xsize ysize;
  (*Graphic_image.draw_image img 0 0;*)
  Printf.printf "Initial seeds...\n";
  flush stdout;
  let cloud = ref (seed_world img population_size) in
  let no_improvement = ref 0 in
  while !no_improvement < 5 do
    let old_best_score = !best_score in
    Printf.printf "Breeding... (no improvement for %d)\n" !no_improvement;
    flush stdout;
    cloud := generate img !cloud population_size;
    if old_best_score = !best_score then
      incr no_improvement
    else
      no_improvement := 0;
  done;
  begin match !best_points with
    Some points ->
      let fo = open_out "result_points" in
      for i = 0 to Array.length points - 1 do
        Printf.fprintf fo "  { %d, %d, 0x%x },\n"
	  points.(i).x points.(i).y points.(i).c
      done;
      close_out fo
  | None ->
      ()
  end;
  Printf.printf "Finished."
