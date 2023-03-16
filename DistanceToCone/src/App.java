public class App {

	private static final double LimelightXFov = 59.6;
	private static final int LimelightXResolution = 320;

	private static final double ConeSideLength = 13.776;
	private static final double ConeBaseLength = 8.375;
	private static final double ConeEdgeAngleOffset = 21.56;
	private static final double ConeBaseAngleOffset = 90;

	public static void main(String[] args) throws Exception {

		System.out.println("Hello, World!");

		//final Point conePosition = GetConePosition(140, 180, new Angle(-77));
		//System.out.println(conePosition.GetX() + ", " +  conePosition.GetY());
	}

	public enum OrientationCase {
		TipDirectlyAway,
		TipDirectlyTowards,
		TipLeftAndAway,
		TipLeftAndTowards,
		TipRightAndAway,
		TipRightAndTowards,
		NoMatch
	}

	static private Angle GetInCameraAngle(final double xPosition, final int xResolution, final double xFov) {

		final double normalizedXPosition = (xPosition - xResolution / 2.0) / (xResolution / 2.0);

		return new Angle(normalizedXPosition * xFov / 2);
	}

	static private OrientationCase GetOrientationCase(final int boundingBoxLeftEdge, final int boundingBoxRightEdge, final Angle coneAngle) {

		final Angle angleToLeftEdge = GetInCameraAngle(boundingBoxLeftEdge, LimelightXResolution, LimelightXFov);
		final Angle angleToRightEdge = GetInCameraAngle(boundingBoxRightEdge, LimelightXResolution, LimelightXFov);
		final Angle angleToCenter = GetInCameraAngle((boundingBoxLeftEdge + boundingBoxRightEdge) / 2.0, LimelightXResolution, LimelightXFov);

		final Angle coneLeftSideAngleFromBase = coneAngle.CopyWithOffset(ConeEdgeAngleOffset);
		final Angle coneRightSideAngleFromBase = coneAngle.CopyWithOffset(-ConeEdgeAngleOffset);
		final Angle coneLeftSideAngleFromTip = coneLeftSideAngleFromBase.CopyWithOffset(180);
		final Angle coneRightSideAngleFromTip = coneRightSideAngleFromBase.CopyWithOffset(180);
		final Angle coneBaseAngleFromLeftCorner = coneAngle.CopyWithOffset(ConeBaseAngleOffset);
		final Angle coneBaseAngleFromRightCorner = coneAngle.CopyWithOffset(-ConeBaseAngleOffset);

		if (coneLeftSideAngleFromBase.ClockwiseOf(angleToLeftEdge) && coneRightSideAngleFromBase.AntiClockwiseOf(angleToRightEdge)) {
			return OrientationCase.TipDirectlyAway;
		}

		if (coneRightSideAngleFromTip.AntiClockwiseOf(angleToLeftEdge) && coneLeftSideAngleFromTip.ClockwiseOf(angleToRightEdge)) {
			return OrientationCase.TipDirectlyTowards;
		}

		if (coneAngle.AntiClockwiseOf(angleToCenter) && coneBaseAngleFromLeftCorner.ClockwiseOf(angleToRightEdge)) {
			return OrientationCase.TipLeftAndAway;
		}

		if (coneAngle.AntiClockwiseOf(angleToCenter) && coneBaseAngleFromLeftCorner.AntiClockwiseOf(angleToRightEdge)) {
			return OrientationCase.TipLeftAndTowards;
		}

		if (coneAngle.ClockwiseOf(angleToCenter) && coneBaseAngleFromRightCorner.AntiClockwiseOf(angleToLeftEdge)) {
			return OrientationCase.TipRightAndAway;
		}

		if (coneAngle.ClockwiseOf(angleToCenter) && coneBaseAngleFromRightCorner.ClockwiseOf(angleToLeftEdge)) {
			return OrientationCase.TipRightAndTowards;
		}

		System.out.println("GetOrinetationCase() wasn't able to match the parameters to a case it knows how to evaluate. boundingBoxLeftEdge=" +
		boundingBoxLeftEdge + ", boundingBoxRightEdge=" + boundingBoxRightEdge + ", coneAngle=" + coneAngle);

		return OrientationCase.NoMatch;
	}

	static public Point GetConePosition(final int boundingBoxLeftEdge, final int boundingBoxRightEdge, final Angle coneAngle) {

		final OrientationCase orientationCase = GetOrientationCase(boundingBoxLeftEdge, boundingBoxRightEdge, coneAngle);

		final Angle angleToLeftEdge = GetInCameraAngle(boundingBoxLeftEdge, LimelightXResolution, LimelightXFov);
		final Angle angleToRightEdge = GetInCameraAngle(boundingBoxRightEdge, LimelightXResolution, LimelightXFov);

		final Angle coneLeftSideAngleFromBase = coneAngle.CopyWithOffset(ConeEdgeAngleOffset);
		final Angle coneRightSideAngleFromBase = coneAngle.CopyWithOffset(-ConeEdgeAngleOffset);
		final Angle coneLeftSideAngleFromTip = coneLeftSideAngleFromBase.CopyInverted();
		final Angle coneRightSideAngleFromTip = coneRightSideAngleFromBase.CopyInverted();
		final Angle coneBaseAngleFromLeftCorner = coneAngle.CopyWithOffset(ConeBaseAngleOffset);
		final Angle coneBaseAngleFromRightCorner = coneAngle.CopyWithOffset(-ConeBaseAngleOffset);

		Angle interiorCameraAngle = Angle.Between(angleToLeftEdge, angleToRightEdge);
		Angle interiorLeftAngle;
		Angle interiorRightAngle;
		double sideOppositeCameraLength;

		switch (orientationCase) {

			case TipDirectlyAway:
				interiorLeftAngle = Angle.Between(coneBaseAngleFromLeftCorner, angleToLeftEdge.CopyInverted());
				interiorRightAngle = Angle.Between(angleToRightEdge.CopyInverted(), coneBaseAngleFromRightCorner);
				sideOppositeCameraLength = ConeBaseLength;
				break;

			case TipDirectlyTowards:
				interiorLeftAngle = Angle.Between(coneBaseAngleFromRightCorner, angleToLeftEdge.CopyInverted());
				interiorRightAngle = Angle.Between(angleToRightEdge.CopyInverted(), coneBaseAngleFromLeftCorner);
				sideOppositeCameraLength = ConeBaseLength;
				break;

			case TipLeftAndAway:
				interiorLeftAngle = Angle.Between(coneRightSideAngleFromTip, angleToLeftEdge.CopyInverted());
				interiorRightAngle = Angle.Between(angleToRightEdge.CopyInverted(), coneRightSideAngleFromBase);
				sideOppositeCameraLength = ConeSideLength;
				break;

			case TipLeftAndTowards:
				interiorLeftAngle = Angle.Between(coneLeftSideAngleFromTip, angleToLeftEdge.CopyInverted());
				interiorRightAngle = Angle.Between(angleToRightEdge.CopyInverted(), coneLeftSideAngleFromBase);
				sideOppositeCameraLength = ConeSideLength;
				break;

			case TipRightAndAway:
				interiorLeftAngle = Angle.Between(coneLeftSideAngleFromBase, angleToLeftEdge.CopyInverted());
				interiorRightAngle = Angle.Between(angleToRightEdge.CopyInverted(), coneLeftSideAngleFromTip);
				sideOppositeCameraLength = ConeSideLength;
				break;

			case TipRightAndTowards:
				interiorLeftAngle = Angle.Between(coneRightSideAngleFromBase, angleToLeftEdge.CopyInverted());
				interiorRightAngle = Angle.Between(angleToRightEdge.CopyInverted(), coneRightSideAngleFromTip);
				sideOppositeCameraLength = ConeSideLength;
				break;

			default:
				System.out.println("No orientation case matched in GetDistanceToCone().");
				return new Point(-1, -1);
		}

		final double leftSideLength = sideOppositeCameraLength / interiorCameraAngle.Sine() * interiorRightAngle.Sine();
		final double rightSideLength = sideOppositeCameraLength / interiorCameraAngle.Sine() * interiorLeftAngle.Sine();

		final Point leftPoint = new Point(angleToLeftEdge.Sine() * leftSideLength, angleToLeftEdge.Cosine() * leftSideLength);
		final Point rightPoint = new Point(angleToRightEdge.Sine() * rightSideLength, angleToRightEdge.Cosine() * rightSideLength);

		Point pointToOffsetFrom;
		Point offset;
		Angle offsetRotation;

		switch (orientationCase) {

			case TipDirectlyAway:
				pointToOffsetFrom = leftPoint;
				offset = new Point(ConeBaseLength / 2, ConeBaseLength / 2);
				offsetRotation = coneAngle;
				break;

			case TipDirectlyTowards:
				pointToOffsetFrom = leftPoint;
				offset = new Point(ConeBaseLength / 2, -ConeBaseLength / 2);
				offsetRotation = coneAngle.CopyInverted();
				break;

			case TipLeftAndAway:
				pointToOffsetFrom = rightPoint;
				offset = new Point(-ConeBaseLength / 2, -ConeBaseLength / 2);
				offsetRotation = coneBaseAngleFromLeftCorner;
				break;

			case TipLeftAndTowards:
				pointToOffsetFrom = rightPoint;
				offset = new Point(-ConeBaseLength / 2, ConeBaseLength / 2);
				offsetRotation = coneBaseAngleFromLeftCorner;
				break;

			case TipRightAndAway:
				pointToOffsetFrom = leftPoint;
				offset = new Point(ConeBaseLength / 2, -ConeBaseLength / 2);
				offsetRotation = coneBaseAngleFromRightCorner;
				break;

			case TipRightAndTowards:
				pointToOffsetFrom = leftPoint;
				offset = new Point(ConeBaseLength / 2, ConeBaseLength / 2);
				offsetRotation = coneBaseAngleFromRightCorner;
				break;

			default:
				System.out.println("no orientation case matched in GetDistanceToCone().");
				return new Point(-1, -1);
		}

		final Point center = pointToOffsetFrom.CopyAndOffset(offset.CopyAndRotate(offsetRotation));

		return center;
	}

}



class Point {

	private final double X;
	private final double Y;

	public double GetX() { return X; }
	public double GetY() { return Y; }

	public Point(double x, double y) {
		X = x;
		Y = y;
	}

	public Point CopyAndOffset(Point offset) {
		return new Point(X + offset.X, Y + offset.Y);
	}

	public Point CopyAndRotate(Angle rotation) {

		final double newX = X * rotation.Cosine() + Y * rotation.Sine();
		final double newY = -X * rotation.Sine() + Y * rotation.Cosine();
		return new Point(newX, newY);
	}

}



class Angle {

	private final double Value;

	public Angle(double value) {

		while (value > 180) {
			value -= 360;
		}

		while (value < -180) {
			value += 360;
		}

		Value = value;
	}

	public Angle CopyWithOffset(double offset) {

		return new Angle(Value + offset);
	}

	public Angle CopyInverted() {
		return CopyWithOffset(180);
	}

	public boolean IsBetween(Angle antiClockwiseLimit, Angle clockwiseLimit) {

		// both on right half
		if (antiClockwiseLimit.Value >= 0 && clockwiseLimit.Value >= 0 && antiClockwiseLimit.Value <= clockwiseLimit.Value) {
			return Value >= antiClockwiseLimit.Value && Value <= clockwiseLimit.Value;

		// both on left half
		} else if (antiClockwiseLimit.Value <= 0 && clockwiseLimit.Value <= 0 && antiClockwiseLimit.Value <= clockwiseLimit.Value) {
			return Value >= antiClockwiseLimit.Value && Value <= clockwiseLimit.Value;

		// acw on left, cw on right
		} else if (antiClockwiseLimit.Value <= 0 && clockwiseLimit.Value >= 0) {
			return Value >= antiClockwiseLimit.Value && Value <= clockwiseLimit.Value;

		// both on right, inverted
		} else if (antiClockwiseLimit.Value >= 0 && clockwiseLimit.Value >= 0 && antiClockwiseLimit.Value >= clockwiseLimit.Value) {
			return (Value <= clockwiseLimit.Value && Value >= -180) || (Value > antiClockwiseLimit.Value && Value <= 180);

		// both on left, inverted
		} else if (antiClockwiseLimit.Value <= 0 && clockwiseLimit.Value <= 0 && antiClockwiseLimit.Value >= clockwiseLimit.Value) {
			return (Value >= antiClockwiseLimit.Value && Value <= 180) || (Value <= clockwiseLimit.Value && Value >= -180);

		// acw of right, cw on left
		} else if (antiClockwiseLimit.Value >= 0 && clockwiseLimit.Value <= 0) {
			return (Value <= clockwiseLimit.Value && Value >= -180) || (Value >= antiClockwiseLimit.Value && Value <= 180);
		}

		System.out.println("IsBetweenInclusive() wasn't able to match the parameters to a case it knows how to evaluate. angle=" 
		+ Value + ", counterClockwiseLimit=" + antiClockwiseLimit.Value + ", clockwiseLimit=" + clockwiseLimit.Value);

		return false;
	}

	public boolean ClockwiseOf(Angle angle) {

		return IsBetween(angle, angle.CopyWithOffset(180));
	}

	public boolean AntiClockwiseOf(Angle angle) {

		return IsBetween(angle.CopyWithOffset(180), angle);
	}

	public double Sine() {

		return Math.sin(Value * Math.PI / 180);
	}
	
	public double Cosine() {

		return Math.cos(Value * Math.PI / 180);
	}

	public static Angle Between(Angle antiClockwiseAngle, Angle clockwiseAngle) {
		
		// both on right half
		if (antiClockwiseAngle.Value >= 0 && clockwiseAngle.Value >= 0 && antiClockwiseAngle.Value <= clockwiseAngle.Value) {
			return new Angle(clockwiseAngle.Value - antiClockwiseAngle.Value);

		// both on left half
		} else if (antiClockwiseAngle.Value <= 0 && clockwiseAngle.Value <= 0 && antiClockwiseAngle.Value <= clockwiseAngle.Value) {
			return new Angle(clockwiseAngle.Value - antiClockwiseAngle.Value);

		// ccw on left, cw on right
		} else if (antiClockwiseAngle.Value <= 0 && clockwiseAngle.Value >= 0) {
			return new Angle(clockwiseAngle.Value - antiClockwiseAngle.Value);

		// both on right, inverted
		} else if (antiClockwiseAngle.Value >= 0 && clockwiseAngle.Value >= 0 && antiClockwiseAngle.Value >= clockwiseAngle.Value) {
			return new Angle(360 - (antiClockwiseAngle.Value - clockwiseAngle.Value));

		// both on left, inverted
		} else if (antiClockwiseAngle.Value <= 0 && clockwiseAngle.Value <= 0 && antiClockwiseAngle.Value >= clockwiseAngle.Value) {
			return new Angle(360 - (antiClockwiseAngle.Value - clockwiseAngle.Value));

		// ccw of right, cw on left
		} else if (antiClockwiseAngle.Value >= 0 && clockwiseAngle.Value <= 0) {
			return new Angle(360 - (antiClockwiseAngle.Value - clockwiseAngle.Value));
		}

		System.out.println("AngleBetween() wasn't able to match the parameters to a case it knows how to evaluate. antiClockwiseAngle=" 
		+ antiClockwiseAngle + ", clockwiseAngle=" + clockwiseAngle.Value);

		return new Angle(0);
	}

}